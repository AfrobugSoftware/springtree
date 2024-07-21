#pragma once
#include <boost/variant2/variant.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/array.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/variant.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>



#include <ranges>
#include <chrono>
#include <vector>
#include <bitset>
#include <cassert>

#include "currency.h"

namespace pof
{
    //represents data in the application
    using namespace std::literals::string_literals;
    using namespace std::literals::chrono_literals;
    namespace ch = std::chrono;
    namespace v2 = boost::variant2;

    namespace base{

        template<typename... Ts>
        class visitor : public Ts...
        {
            using Ts::operator()...;
        };

        template<typename... Ts>
        visitor(Ts...) -> visitor<Ts...>;

        class data{
        public:
            enum class state {
                CREATED,
                MODIFIED,
                HIDDEN,
                MAX_STATE
            };
            
            enum class kind : std::uint8_t {
                int32,
                int64,
                uint32,
                uint64,
                float32,
                float64,
                datetime,
                text,
                blob,
                uuid,
                currency,
                null //null kind represents a datum that is null, used for tigther packing
            };

            using clock_t = std::chrono::system_clock;
            using datetime_t = clock_t::time_point;
            using text_t = std::string;
            using blob_t = std::vector<std::uint8_t>;
            using metadata_t = std::vector<kind>;
            using duuid_t = boost::uuids::uuid;
            using currency_t = pof::base::currency;

            using data_t = v2::variant<
                std::int32_t,
                std::int64_t,
                std::uint32_t,
                std::uint64_t,
                float,
                double,
                datetime_t,
                text_t,
                blob_t,
                duuid_t,
                currency_t
            >;
            
            using state_t = std::bitset<static_cast<size_t>(std::underlying_type_t<state>(state::MAX_STATE))>;
            using update_t = std::bitset<256>; //update flags
            using row_t = std::pair<std::vector<data_t>, std::pair<state_t, update_t>>;
            using table_t = std::vector<row_t>;
            
            using iterator = table_t::iterator;
            using const_iterator = table_t::const_iterator;
            using value_type = table_t::value_type;

            //ctors
            data();
            data(size_t count);
            ~data();

            inline data(const data& rhs) : 
                metadata(rhs.metadata),
                value(rhs.value),
                bModified(rhs.bModified),
                created(rhs.created),
                modified(rhs.modified)
            {}
            inline data(data&& rhs) noexcept :
                metadata(std::move(rhs.metadata)),
                value(std::move(rhs.value)), 
                bModified(rhs.bModified),
                created(rhs.created),
                modified(rhs.modified)
            {}

            data& operator=(const data& rhs);
            data& operator=(data&& rhs) noexcept;

            //metadata functions
            inline void set_metadata(const metadata_t& md) { metadata = md; }
            constexpr const metadata_t& get_metadata() const { return metadata; }
            constexpr metadata_t& get_metadata() { return metadata; }


            void insert(row_t&& row);
            void insert(const typename row_t::first_type& vals);
            void insert(const typename row_t::first_type& vals, const typename row_t::second_type& st);
            void update(const typename row_t::first_type::value_type& d, size_t idx, size_t idy);


            void emplace(typename row_t::first_type&& vals);
            const row_t& at(size_t i ) const; //throws std::out_of_range if out of bands

            constexpr size_t size() const { return value.size(); }
            inline void clear() { value.clear(); }
            void clear_state(state s);
            void clear_state(size_t x, state s);

            void set_state(state s);
            void set_state(size_t x, state s);

            bool test_state(size_t x, state s);

            inline void reserve(size_t size) { value.reserve(size); }
            inline void resize(size_t size) { value.resize(size); }
            void shrink_to_fit();

            inline const auto& back() const { return value.back(); }
            inline const auto& front() const { return value.front(); }

            const row_t& operator[](size_t i) const;
            row_t& operator[](size_t i);
            bool operator==(const data& rhs) const = default;

            //get the underlying table
            inline const table_t& tab() const { return value; }
            inline table_t& tab()  { return value; }

            inline iterator begin() { return value.begin(); }
            inline const_iterator begin() const { return value.begin(); }
            inline iterator end() { return value.end(); }
            inline const_iterator end() const { return value.end();}

            //modifying the wors

            //TODO: test for the arch of the system, should not be built on a 32 bit machine?
            //or change the datatype on a 32 bit machine ?
            //how would the server handle different machine architecture?
            template<typename Archive>
            void save(Archive& ar, const unsigned int version = 1) const {
                assert(!metadata.empty());
                if (value.empty()) return; // nothing to serialise

                //forms the header of the data:
                //write time created and time last modified
                ar & static_cast<std::uint32_t>(version); //write the version
                ar & created.time_since_epoch().count();
                ar & modified.time_since_epoch().count();
                ar & static_cast<std::uint32_t>(value.size()); //size of rows
                ar & static_cast<std::uint32_t>(metadata.size()); //size of cols also size of meta
                for (auto& k : metadata) {
                    ar & static_cast<std::underlying_type_t<kind>>(k);
                }

;                for (auto& row : value) 
                 {
                        auto& [r, s] = row;
                        const size_t size = r.size();
                        std::bitset<32> aval_row;
                        if (!s.first.test(static_cast<std::underlying_type_t<state>>(state::CREATED)) && !s.first.test(static_cast<std::underlying_type_t<state>>(state::MODIFIED))) {
                            continue;
                        }

                        for (std::uint8_t i = 0; i < size; i++) {
                            auto k = metadata[i];
                            auto& d = r[i];
                            if (d.index() != static_cast<std::underlying_type_t<kind>>(k)) continue; //skip null cols

                            switch (k)
                            {
                            case pof::base::data::kind::int32:
                                 ar & i;
                                 ar & static_cast<std::underlying_type_t<kind>>(k);
                                 ar & boost::variant2::get<std::int32_t>(d);
                                break;
                            case pof::base::data::kind::int64:
                                ar & i;
                                ar & static_cast<std::underlying_type_t<kind>>(k);
                                ar & boost::variant2::get<std::int64_t>(d);
                                break;
                            case pof::base::data::kind::uint32:
                                ar & i;
                                ar & static_cast<std::underlying_type_t<kind>>(k);
                                ar & boost::variant2::get<std::uint32_t>(d);
                                break;
                            case pof::base::data::kind::uint64:
                                ar & i;
                                ar & static_cast<std::underlying_type_t<kind>>(k);
                                ar & boost::variant2::get<std::uint64_t>(d);
                                break;
                            case pof::base::data::kind::float32:
                                ar & i;
                                ar & static_cast<std::underlying_type_t<kind>>(k);
                                ar & boost::variant2::get<float>(d);
                                break;
                            case pof::base::data::kind::float64:
                                ar & i;
                                ar & static_cast<std::underlying_type_t<kind>>(k);
                                ar & boost::variant2::get<double>(d);
                                break;
                            case pof::base::data::kind::datetime:
                            {
                                auto& t = boost::variant2::get<datetime_t>(d);
                                ar & i;
                                ar & static_cast<std::underlying_type_t<kind>>(k);
                                ar & t.time_since_epoch().count();
                                break;
                            }
                            case pof::base::data::kind::text:
                            {
                                ar & i;
                                ar & static_cast<std::underlying_type_t<kind>>(k);
                                ar & boost::variant2::get<text_t>(d);
                                break;
                            }
                            case pof::base::data::kind::blob:
                                ar & i;
                                ar & static_cast<std::underlying_type_t<kind>>(k);
                                ar & boost::variant2::get<blob_t>(d);
                                break;
                            case pof::base::data::kind::uuid:
                            {
                                //the array might be serializing a pointer, need to test
                                ar & i;
                                ar & static_cast<std::underlying_type_t<kind>>(k);
                                ar & boost::variant2::get<duuid_t>(d).data;
                                break;
                            }
                            case pof::base::data::kind::currency:
                                ar & i;
                                ar & static_cast<std::underlying_type_t<kind>>(k);
                                ar & boost::variant2::get<currency_t>(d).data();
                                break;
                            default:
                                break;
                            }
                        }
                }
            }

         
            template<typename Archiver>
            void load(Archiver& ar, const unsigned int version = 1){
                //read the header
                clock_t::duration::rep rep = 0; 
                std::uint32_t rowsize = 0, colsize = 0, ver = 0;
                ar >> ver; //read in the version .. check version type
                ar >> rep;
                created = datetime_t(clock_t::duration(rep));
                ar >> rep;
                modified = datetime_t(clock_t::duration(rep));

                ar >> rowsize;
                value.reserve(rowsize);
                ar >> colsize; //save for later
                //metadata size
                metadata.resize(colsize);
                std::uint8_t k = 0;
                for (int i = 0; i < metadata.size(); i++) {
                    ar >> k;
                    metadata[i] = static_cast<kind>(k);
                }
                
                //maybe I do not need to loop this by row size
                //read until the stream is at end of file
                value.emplace_back(row_t{});
                std::reference_wrapper<row_t::first_type> r = value.back().first;
                r.get().resize(colsize);

               std::uint8_t col = 0;
               std::uint8_t old_col = 0;
               std::uint8_t s = 0;
               size_t count = value.size();
               while(count <= rowsize) {
                    try {
                        ar >> col;
                        if (col < old_col) {
                            value.emplace_back(row_t{});
                            count = value.size();
                            r = value.back().first;
                            r.get().resize(colsize);
                        }
                        old_col = col;
                        ar >> s;
                        kind k = static_cast<kind>(s);
                        value.back().second.second.set(col);
                        switch (k)
                        {
                        case pof::base::data::kind::int32:
                        {
                            std::int32_t temp = 0;
                            ar >> temp;
                            r.get()[col] = temp;
                            break;
                        }
                        case pof::base::data::kind::int64:
                        {
                            std::int64_t temp = 0;
                            ar >> temp;
                            r.get()[col] = temp;
                            break;
                        }
                        case pof::base::data::kind::uint32:
                        {
                            std::uint32_t temp = 0;
                            ar >> temp;
                            r.get()[col] = temp;
                            break;
                        }
                        case pof::base::data::kind::uint64:
                        {
                            std::uint64_t temp = 0;
                            ar >> temp;
                            r.get()[col] = temp;
                            break;
                        }
                        case pof::base::data::kind::float32:
                        {
                            float temp = 0.0f;
                            ar >> temp;
                            r.get()[col] = temp;
                            break;
                        }
                        case pof::base::data::kind::float64:
                        {
                            double temp = 0.0;
                            ar >> temp;
                            r.get()[col] = temp;
                            break;
                        }
                        case pof::base::data::kind::datetime:
                        {
                            clock_t::duration::rep rep = 0;
                            ar >> rep;
                            r.get()[col] = datetime_t(clock_t::duration(rep));
                            break;
                        }
                        case pof::base::data::kind::text:
                        {
                            text_t temp{};
                            ar >> temp;
                            r.get()[col] = std::move(temp);
                            break;
                        }
                        case pof::base::data::kind::blob:
                        {
                            blob_t temp{};
                            ar >> temp;
                            r.get()[col] = std::move(temp);
                            break;
                        }
                        case pof::base::data::kind::uuid:
                        {
                            duuid_t uuid = { 0 };
                            ar >> uuid.data;
                            r.get()[col] = (std::move(uuid));
                        }
                        case pof::base::data::kind::currency:
                        {
                            pof::base::currency::cur_t data = { 0 };
                            ar >> data;
                            r.get()[col] = (pof::base::currency(std::move(data)));
                        }
                        default:
                            break;
                        }
                    }
                    catch (const std::exception& exp) {
                        //hopefully this would be a read passed eof error
                       // spdlog::error(exp.what());
                        break;
                    }
                }
              
            }
            iterator erase(const_iterator iter) { return value.erase(iter); }

            inline constexpr datetime_t tsCreated() const { return created; }
            inline constexpr datetime_t tsModified() const { return modified; }

            inline constexpr void tsCreated(const datetime_t& dt) { created = dt; }
            inline constexpr void tsModified(const datetime_t& dt) { modified = dt; }
            inline constexpr bool empty() const { return value.empty(); }


            //variant visitor to get the the current value of the variant
           
        private:
            friend class boost::serialization::access;
            BOOST_SERIALIZATION_SPLIT_MEMBER()
            //serialise flags
            bool bModified; //serialise only modified rows


            datetime_t created;
            datetime_t modified;

            metadata_t metadata; //holds type information of the varaints
            table_t value;

        };

        class dataView : public std::ranges::view_interface<dataView>
        {
        public:
            dataView(const pof::base::data& data_) : mBegin(data_.begin()),
                mEnd(data_.end())
            {};
            dataView(data::const_iterator beg, data::const_iterator en) : 
                mBegin(beg),
                mEnd(en)
            {}

        private:
            data::const_iterator mEnd;
            data::const_iterator mBegin;
        };

    };
};