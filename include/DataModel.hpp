#pragma once

#include <functional>
#include <boost/unordered_map.hpp>

#include <wx/dataview.h>
#include "serialiser.h"

namespace ab {
	template<grape::FusionStruct T>
	class DataModel : public wxDataViewVirtualListModel,
		public std::vector<boost::fusion::vector<std::bitset<boost::mpl::size<T>::value>, std::array<wxDataViewItemAttr, boost::mpl::size<T>::value>, T>>
	{
	public:
		using row_t = boost::fusion::vector<std::bitset<boost::mpl::size<T>::value>,
			std::array<wxDataViewItemAttr, boost::mpl::size<T>::value>, T>;
		using vec_base = std::vector<row_t>;
		using vec_base::operator[];
		using specialcol_t = std::pair<std::function<void(wxVariant&, unsigned int row, unsigned int col)>,
			std::function<bool(const wxVariant&, unsigned int row, unsigned int col)>>;
		using sp_value = boost::unordered_map<size_t, specialcol_t>::value_type::second_type;

		static size_t FromDataViewItem(const wxDataViewItem& item) {
			return (reinterpret_cast<size_t>(item.GetID()) - 1);
		}

		static wxDataViewItem ToDataViewItem(size_t i) {
			return wxDataViewItem(reinterpret_cast<void*>(i + 1));
		}

		
		DataModel(size_t count = 0) : wxDataViewVirtualListModel(count), vec_base(count) {}
		virtual ~DataModel() = default;

		virtual bool GetAttrByRow(unsigned int row, unsigned int col, wxDataViewItemAttr& attr) const override
		{
			if (vec_base::empty()) return false;
			const auto& r = (*this)[row];
			auto& atr = boost::fusion::at_c<1>(r);
			attr = atr[col];
			return true;
		}

		virtual bool IsEnabledByRow(unsigned int row, unsigned int col) const override
		{
			return true; //alway enabled ?
		}

		virtual unsigned int GetCount() const override {
			return vec_base::size();
		}

		virtual unsigned int GetRow(const wxDataViewItem& item) const override
		{
			return FromDataViewItem(item);
		}

		std::optional<const T&> GetValue(size_t row) const {
			if (vec_base::empty()) return std::nullopt;
			return boost::fusion::at_c<2>((*this)[row]);
		}

		std::optional<T&> GetValue(size_t row) {
			if (vec_base::empty()) return std::nullopt;
			return boost::fusion::at_c<2>((*this)[row]);
		}

		void Reload( const std::vector<T>& items) {
			Clear();
			for (auto&& i : items) {
				typename vec_base::value_type v{};
				boost::fusion::at_c<2>(v) = std::move(i);
			}
			Reset(items.size());
		}


		void Clear() {
			vec_base::clear();
			Cleared();
		}

		virtual void GetValueByRow(wxVariant& variant, unsigned int row, unsigned int col) const override
		{
			typedef boost::mpl::range_c<unsigned, 0, boost::mpl::size<T>::value> range;
			
			auto iter = mSpecialColMap.find(col);
			if (iter != mSpecialColMap.end()) {
				auto& f = iter->second.first;
				if (f) {
					f(variant, row, col);
				}
				return;
			}
			

			if (col > col_count || vec_base::empty()) return;
			auto& r = boost::fusion::at_c<2>((*this)[row]);
			boost::fusion::for_each(range(), [&](auto i) {
				using constant = std::decay_t<decltype(i)>;
				using arg_type = std::decay_t<decltype(boost::fusion::at<constant>(r))>;
				if (constant::value != col) return;

				auto& v = boost::fusion::at_c<constant::value>(r);
				if constexpr (std::is_same_v<std::chrono::year_month_day, arg_type>)
				{
					std::chrono::sys_days days{v};
					variant =  wxVariant(wxDateTime(days.time_since_epoch().count()));
				}
				else if constexpr (std::is_same_v<std::chrono::system_clock::time_point, arg_type>)
				{
					variant = wxVariant(wxDateTime(v.time_since_epoch().count()));
				}
				else if constexpr (std::is_enum_v<arg_type>)
				{
					variant = wxVariant("");
				}
				else if constexpr (std::is_same_v<arg_type, pof::base::currency>) {
					double amount = static_cast<double>(v);
					variant = wxVariant(std::format("N {:.2f}", amount));
				}
				else if constexpr (std::is_integral_v<arg_type>){
					if constexpr (sizeof(arg_type) == 4) {
						variant = wxVariant(v);
					}
					else {
						variant = wxVariant(wxLongLong(v));
					}
				}
				else if constexpr (std::is_same_v<std::string, arg_type>) {
					variant = wxVariant(v);
				}

			});

		}

		virtual bool SetValueByRow(const wxVariant& variant, unsigned int row, unsigned int col) {
			try {
				typedef boost::mpl::range_c<unsigned, 0, boost::mpl::size<T>::value> range;
				auto iter = mSpecialColMap.find(col);
				if (iter != mSpecialColMap.end()) {
					auto& f = iter->second.second;
					if (f) {
						return f(variant, row, col);
					}
					return false;
				}

				if (col > col_count || vec_base::empty()) return false;
				auto& r = boost::fusion::at_c<2>((*this)[row]); 
					boost::fusion::for_each(range(), [&](auto i) {
					using constant = std::decay_t<decltype(i)>;
					using arg_type = std::decay_t<decltype(boost::fusion::at<constant>(r))>;
					if (constant::value != col) return;


					auto& v = boost::fusion::at_c<constant::value>(r);
					if constexpr (std::is_same_v<std::chrono::year_month_day, arg_type>)
					{
						auto date = variant.GetDateTime();
						std::chrono::sys_days days =
							std::chrono::time_point_cast<std::chrono::sys_days::duration> (std::chrono::system_clock::from_time_t(date.GetTicks()));

						v = std::chrono::year_month_day{ days };
					}
					else if constexpr (std::is_same_v<std::chrono::system_clock::time_point, arg_type>)
					{
						auto date = variant.GetDateTime();
						v = std::chrono::system_clock::from_time_t(date.GetTicks());
					}
					else if constexpr (std::is_enum_v<arg_type>)
					{
					}
					else if constexpr (std::is_integral_v<arg_type>){
						if constexpr (sizeof(arg_type) == 4) {
							if constexpr (std::is_signed_v<arg_type>) {
								v = variant.GetLong();
							}
							else {
								v = static_cast<arg_type>(variant.GetLong());
							}
						}
						else {
							if constexpr (std::is_signed_v<arg_type>) {
								v = variant.GetLongLong().GetValue();
							}
							else {
								v = variant.GetULongLong().GetValue();
							}
						}
					}
					else if constexpr (std::is_same_v<arg_type, pof::base::currency>) {
						auto string = variant.GetString().ToStdString();
						v = pof::base::currency(string);
					}
					else if constexpr (std::is_same_v<std::string, arg_type>){
						v = variant.GetString().ToStdString();
					}
					});
					return true;
			}
			catch (const std::exception& exp) {
				spdlog::error(exp.what());
				return false;
			}
		}

		void AddSpecialCol(sp_value&& value, size_t col) {
			auto&& [iter, in] = mSpecialColMap.insert({ col, std::forward<sp_value>(value) });
			if (!in) {
				iter->second = std::forward<sp_value>(value);
			}
		}

	private:
		constexpr static const size_t col_count = boost::mpl::size<T>::value;
		std::vector<wxDataViewItemAttr> mExtrattrs;
		boost::unordered_map<size_t, specialcol_t> mSpecialColMap;


	};
};