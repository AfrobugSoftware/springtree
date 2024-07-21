#include "Data.h"

pof::base::data::data()
: bModified(false){
}

//not set metadata
pof::base::data::data(size_t count)
: value(count),
  bModified(false),
  created(pof::base::data::clock_t::now()),
  modified(pof::base::data::clock_t::now())
{}

pof::base::data::~data()
{

}

pof::base::data& pof::base::data::operator=(const data& rhs)
{
	metadata = rhs.metadata;
	value = rhs.value;
	bModified = rhs.bModified;
	created = rhs.created;
	modified = rhs.modified;

	return (*this);
}

pof::base::data& pof::base::data::operator=(data&& rhs) noexcept
{
	metadata = std::move(rhs.metadata);
	value = std::move(rhs.value);
	bModified = rhs.bModified;
	created = rhs.created;
	modified = rhs.modified;

	return (*this);
}

void pof::base::data::insert(row_t&& row)
{
	assert(row.first.size() == metadata.size()); //not less than or greater than the row count
	modified = clock_t::now();
	constexpr size_t pos = (std::underlying_type_t<state>)state::CREATED;

	value.emplace_back(std::forward<row_t>(row));
	value.back().second.first.set(pos);
}

void pof::base::data::insert(const typename row_t::first_type& vals)
{
	assert(vals.size() == metadata.size()); //not less than or greater than the row count
	modified = clock_t::now();
	constexpr size_t pos = (std::underlying_type_t<state>)state::CREATED;

	value.push_back({ vals, {state_t{}, update_t{}} });
	value.back().second.first.set(pos);
}

void pof::base::data::insert(const typename row_t::first_type& vals, const typename row_t::second_type& st)
{
	assert(vals.size() == metadata.size()); //not less than or greater than the row count
	modified = clock_t::now();
	constexpr size_t pos = (std::underlying_type_t<state>)state::CREATED;
	value.push_back({ std::forward<const row_t::first_type&>(vals),  st});
	value.back().second.first.set(pos);
}

void pof::base::data::update(const typename row_t::first_type::value_type& d, size_t idx, size_t idy)
{
	if (idx > value.size()) throw std::out_of_range("idx is out of range");
	auto& row = value[idx];
	if (idy > row.first.size()) throw std::out_of_range("idy is out of range");

	row.first[idy] = d;
	row.second.first.set((std::underlying_type_t<state>)state::MODIFIED);
	bModified = true;
	modified = clock_t::now();
}

void pof::base::data::emplace(typename row_t::first_type&& vals)
{
	auto& b = value.emplace_back(row_t{});
	b.first = std::forward<row_t::first_type>(vals);

	modified = clock_t::now();
	constexpr size_t pos = (std::underlying_type_t<state>)state::CREATED;
	b.second.first.set(pos);
}

const pof::base::data::row_t & pof::base::data::at(size_t i) const
{
	return value.at(i);
}

void pof::base::data::clear_state(state s)
{
	for (auto& row : value) {
		row.second.first.set(static_cast<std::underlying_type_t<state>>(s), false);
	}
}

void pof::base::data::clear_state(size_t x, state s)
{
	if (x > value.size()) throw std::runtime_error("Row does not exist");;
	value[x].second.first.set(static_cast<std::underlying_type_t<state>>(s), false);
}

void pof::base::data::set_state(state s)
{
	for (auto& row : value) {
		row.second.first.set(static_cast<std::underlying_type_t<state>>(s));
	}
}

void pof::base::data::set_state(size_t x, state s)
{
	if (x > value.size()) throw std::runtime_error("Row does not exist");;
	value[x].second.first.set(static_cast<std::underlying_type_t<state>>(s), false);
}

bool pof::base::data::test_state(size_t x, state s)
{
	if (x > value.size()) throw std::runtime_error("Row does not exist");
	return value[x].second.first.test(static_cast<std::underlying_type_t<state>>(s));
}

void pof::base::data::shrink_to_fit()
{
	value.shrink_to_fit();
}

const pof::base::data::row_t& pof::base::data::operator[](size_t i) const
{
	if (i > value.size()) throw std::out_of_range("Row index is out of range");
	return value[i];
}

pof::base::data::row_t& pof::base::data::operator[](size_t i)
{
	// TODO: insert return statement here
	if (i > value.size()) throw std::out_of_range("Row index is out of range");
	return value[i];
}
