#pragma once
#include <list>
#include <memory>
#include <optional>

#include "Data.h"
#include "data_tuple.h"
namespace pof
{
	namespace base {

		template<typename T>
		concept identifer = std::is_copy_assignable_v<T> 
				|| std::is_default_constructible_v<T> || std::is_trivially_destructible_v<T>
				|| std::is_trivially_copy_assignable_v<T>;

		template<identifer T>
		class basenode
		{
		public:

			using row_t = pof::base::data::row_t;
			using identifier_type = T;
			basenode() = default;
			basenode(const T& identify) : m_identifyer(identify) {}
			virtual ~basenode() {}
			virtual std::optional<pof::base::data::data_t> get_value(const T& ident, size_t col, size_t row = 0) const = 0;
			virtual bool set_value(const T& ident,const pof::base::data::data_t& dat
				,size_t col, size_t row = 0) = 0;


			std::shared_ptr<basenode<T>> get_parent() const noexcept {
				return m_parent.lock(); //returns null if not pointing to a valid shared pointer
			}

			void set_parent(std::shared_ptr<basenode<T>> ptr) {
				m_parent = ptr;
			}

			constexpr const T& get_identifier() const { return m_identifyer; }
			void set_identifier(T&& ident) { m_identifyer = std::forward<T>(ident); }
		protected:
			T m_identifyer;
			std::weak_ptr<basenode> m_parent;
		};

		template<identifer T>
		class node : public basenode<T>
		{
		public:
			using base_t = basenode<T>;

			node() = default;
			node(const T& ident) : basenode<T>(ident) {}
			virtual ~node() {}
			virtual bool set_value(const T& ident, const pof::base::data::data_t& dat,
				size_t col, size_t row = 0) override {
				if (ident == basenode<T>::m_identifyer) {
					if (col >= m_row.first.size()) {
						return false;
					}
					m_row.first[col] = dat;
					return true;
				}
				bool done = false;
				for (auto& child : m_children) {
					done = child->set_value(ident, dat, col, row);
					if (done) return done;
				}
				return false;
			}

			virtual std::optional<pof::base::data::data_t> get_value(const T& ident, size_t col, size_t row = 0) const override {
				if (ident == basenode<T>::m_identifyer) {
					if (col >= m_row.first.size()) {
						return {};
					}
					auto& [v, s] = m_row;
					return v[col];
				}

				for (auto& child : m_children) {
					auto p = child->get_value(ident, col, row);
					if (p) return p; //finds any data, 
				}
				return {}; //cannot find data so return an empty optional ? 
			}

			bool add_child(std::shared_ptr<basenode<T>> ptr) {
				//revent 2 children from having the same idne
				auto cur = m_children.begin();
				while (cur != m_children.end()) {
					if ((*cur)->get_identifier() == ptr->get_identifier()) {
						//identifer already exists 
						return false;
					}
					cur = std::next(cur);
				}
				m_children.push_back(ptr);
				return true;
			}

			bool remove_child(std::shared_ptr<basenode<T>> ptr){
				auto cur = m_children.begin();
				while (cur != m_children.end()) {
					if (*cur == ptr) {
						m_children.erase(cur);
						return true;
					}
					cur = std::next(cur);
				}
				return false;
			}

		protected:
			typename basenode<T>::row_t m_row;
			std::list<std::shared_ptr<basenode<T>>> m_children;
		};


		template<identifer T>
		class leaf : public basenode<T>
		{
		public:
			//or shouold be a boolean that return
			using base_t = basenode<T>;
			leaf() = default;
			leaf(const T& ident) : basenode<T>(ident) {}
			virtual ~leaf() {}

			template<typename... args>
			void make_meta() {
				pof::base::adapt<args...>(m_datastore);
			}

			virtual bool set_value(const T& ident, const pof::base::data::data_t& dat,
				size_t col, size_t row = 0) override {
				if (ident != basenode<T>::m_identifyer || col >= m_datastore.get_metadata().size()) {
					return false;
				}
				if (row >= m_datastore.size()) return false;
				auto& [r, s] = m_datastore[row];
			
				r[col] = dat;
				return true;
			}

			virtual std::optional<pof::base::data::data_t> get_value(const T& ident, size_t col, size_t row = 0) const override {
				if (ident != basenode<T>::m_identifyer || col >= m_datastore.get_metadata().size()) {
					return {};
				}
				if (row >= m_datastore.size()) return {};
				const auto& [r, s] = m_datastore[row];
				return r[col];
			}
		protected:
			pof::base::data m_datastore;
		};
	}
};