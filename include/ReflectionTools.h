#pragma once
#include "Reflection.h"

template<ReflectionStruct RS> bool operator==(const RS& a, const RS& b)
{
    const auto mbrs = std::remove_reference_t<RS>::get_members();

	const auto call_fn = [&](const auto&...mbr)
	{
		return ((a.*mbr.pointer==b.*mbr.pointer)&& ...);
	};
	return std::tuple_size<decltype(mbrs)>()==0 || std::apply(call_fn, mbrs);
}
template<ReflectionStruct RS> bool operator!=(const RS& a, const RS&b)
{
	return !(a==b);
}
