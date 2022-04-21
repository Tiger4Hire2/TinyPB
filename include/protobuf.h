#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <span>
#include <map>
#include <assert.h>
#include <numeric>
#include <functional>
#include "Reflection.h"
#include "traits.h"
#include "BasicWrapper.h"
#include "Enum.h"
#include "Overloaded.h"

// rules for declaring structures/members 
class NonVarIntTag{};
class FixedTag{};
class SignedTag{};

template<class T, class Tag = NonVarIntTag>
class BasicTypeWrapper : public BasicWrapper<T> {
    public:
    using BasicWrapper<T>::BasicWrapper;
    using BasicWrapper<T>::operator+;   // expose as required
    using BasicWrapper<T>::operator-;
    using tag_type = Tag;
};

template<typename T> concept NonStringContainer =
    requires { typename T::value_type;} &&  // element must be default constructable
    requires(T t) { t.begin(); } &&
    requires(T t) { t.end(); } &&
    requires(T t) { t.push_back(typename T::value_type{}); } &&
    !std::is_same_v<std::string, T>;
	//members_are_ordered<T>();


template <class T> using FixedInt = BasicTypeWrapper<T,FixedTag>;
template <class T> using SignedInt = BasicTypeWrapper<T,SignedTag>;

template<class T> class is_signed: public std::false_type {};
template<class T> class is_signed<SignedInt<T>>: public std::true_type {};
template<class T> constexpr bool is_signed_v{is_signed<T>::value};

template<class T> constexpr bool is_non_string_container_v{false};
template<NonStringContainer T> constexpr bool is_non_string_container_v<T>{true};
static_assert(!is_non_string_container_v<std::string>);
static_assert(is_non_string_container_v<std::vector<int>>);
static_assert(is_non_string_container_v<std::vector<int>>);

// rules for encoding for transmition

template<typename T> concept EnumType = std::is_enum_v<T>;
enum class WireType { VARINT, FIXED32=5, FIXED64=1, DELIMITED=2};
template<class T> constexpr WireType OnWireType();
template<ProtoStruct PS> constexpr WireType OnWireType() { return WireType::DELIMITED; }
template<NonStringContainer PS> constexpr WireType OnWireType() { return WireType::DELIMITED; }

template<EnumType T> constexpr WireType OnWireType() { return WireType::VARINT; }
template<> constexpr WireType OnWireType<bool>() { return WireType::VARINT; }
template<> constexpr WireType OnWireType<int8_t>() { return WireType::VARINT; }
template<> constexpr WireType OnWireType<int16_t>() { return WireType::VARINT; }
template<> constexpr WireType OnWireType<int32_t>() { return WireType::VARINT; }
template<> constexpr WireType OnWireType<int64_t>() { return WireType::VARINT; }
template<> constexpr WireType OnWireType<uint8_t>() { return WireType::VARINT; }
template<> constexpr WireType OnWireType<uint16_t>() { return WireType::VARINT; }
template<> constexpr WireType OnWireType<uint32_t>() { return WireType::VARINT; }
template<> constexpr WireType OnWireType<uint64_t>() { return WireType::VARINT; }
// Signed
template<> constexpr WireType OnWireType<SignedInt<bool>>() { return WireType::VARINT; }
template<> constexpr WireType OnWireType<SignedInt<uint8_t>>() { return WireType::VARINT; }
template<> constexpr WireType OnWireType<SignedInt<int8_t>>() { return WireType::VARINT; }
template<> constexpr WireType OnWireType<SignedInt<uint16_t>>() { return WireType::VARINT; }
template<> constexpr WireType OnWireType<SignedInt<int16_t>>() { return WireType::VARINT; }
template<> constexpr WireType OnWireType<SignedInt<uint32_t>>() { return WireType::VARINT; }
template<> constexpr WireType OnWireType<SignedInt<int32_t>>() { return WireType::VARINT; }
template<> constexpr WireType OnWireType<SignedInt<uint64_t>>() { return WireType::VARINT; }
template<> constexpr WireType OnWireType<SignedInt<int64_t>>() { return WireType::VARINT; }
// Fixed
template<> constexpr WireType OnWireType<FixedInt<bool>>() { return WireType::FIXED32; }
template<> constexpr WireType OnWireType<FixedInt<uint8_t>>() { return WireType::FIXED32; }
template<> constexpr WireType OnWireType<FixedInt<int8_t>>() { return WireType::FIXED32; }
template<> constexpr WireType OnWireType<FixedInt<uint16_t>>() { return WireType::FIXED32; }
template<> constexpr WireType OnWireType<FixedInt<int16_t>>() { return WireType::FIXED32; }
template<> constexpr WireType OnWireType<FixedInt<uint32_t>>() { return WireType::FIXED32; }
template<> constexpr WireType OnWireType<FixedInt<int32_t>>() { return WireType::FIXED32; }
template<> constexpr WireType OnWireType<FixedInt<uint64_t>>() { return WireType::FIXED64; }
template<> constexpr WireType OnWireType<FixedInt<int64_t>>() { return WireType::FIXED64; }
template<> constexpr WireType OnWireType<float>() { return WireType::FIXED32; }
template<> constexpr WireType OnWireType<double>() { return WireType::FIXED64; }
// BString
template<> constexpr WireType OnWireType<std::string>() { return WireType::DELIMITED; }
template<> constexpr WireType OnWireType<char[]>() { return WireType::DELIMITED; }


// data types
using DataBlock = std::vector<std::byte>;
using ConstDataBlock = std::span<const std::byte>;
inline ConstDataBlock as_const(const DataBlock& v) {return ConstDataBlock{v}; }


// rules for writing a structure as a DataBlock

inline DataBlock& operator<<(DataBlock& tgt, std::byte v)
{
    tgt.push_back(v);
    return tgt;
}

inline ConstDataBlock operator>>(ConstDataBlock src, std::byte& v)
{
    v = src[0];
    return ConstDataBlock{src.subspan(1)};
}

template<class T>
inline std::byte EncodeField(FieldID id)
{
    return std::byte{static_cast<std::byte>(id<<3) | static_cast<std::byte>(OnWireType<T>())};
}


template<class T>
inline void WriteAsVarint(DataBlock& tgt, T&& obj)
{
    std::uint64_t val = static_cast<std::uint64_t>(obj);
    do
    {
        std::byte next = static_cast<std::byte>(val)&std::byte{0x7F};
        val >>= 7;
        if (val)
            next |= std::byte{0x80};
        tgt << next;
    } while (val);
}

template<class T>
inline void WriteAsSignedVarint32(DataBlock& tgt, T&& obj)
{
    const std::int32_t val = static_cast<std::int32_t>(obj);
    const std::uint32_t sval = (val>=0) ? (val<<1) : (uint32_t)(std::abs(val+1)<<1)|1;
    WriteAsVarint(tgt, sval);
}

template<class T>
inline void WriteAsSignedVarint64(DataBlock& tgt, T&& obj)
{
    std::int64_t val = static_cast<std::int64_t>(obj);
    const std::uint64_t sval = (obj>=0) ? (val<<1) : ((uint64_t)((std::abs(val+1))<<1)|1);
    WriteAsVarint(tgt, sval);
}


template<class T>
inline void WriteAsFixed32(DataBlock& tgt, T&& obj)
{
    std::uint32_t val = static_cast<std::int32_t>(obj);
    const auto bytes = std::as_bytes(std::span{&val, 1});
    for(const auto& b: bytes)
        tgt << b;
}

template<class T>
inline void WriteAsFixed64(DataBlock& tgt, T&& obj)
{
    std::uint64_t val = static_cast<std::int32_t>(obj);
    const auto bytes = std::as_bytes(std::span{&val, 1});
    for(const auto& b: bytes)
        tgt << b;
}

template<class T>
inline void WriteDelimitedBytes(DataBlock& tgt, T&& obj)
{
    static_assert(sizeof(obj[0])==1);
    const auto as_span = std::span{obj.data(), obj.size()};
    const auto as_byte_span = std::as_bytes(as_span);
    WriteAsVarint(tgt, (int)as_byte_span.size());
    for (const std::byte& e: as_byte_span)
        tgt << e;
}

template<class T> class is_string: public std::false_type{};
template<class CHAR, class ALLOC> class is_string<std::basic_string<CHAR, ALLOC>>: public std::true_type{};

template<class T>
inline DataBlock& operator<<(DataBlock& tgt, const T& obj)
{
    using this_type = std::remove_const_t<std::remove_reference_t<T>>;
    constexpr auto type = OnWireType<this_type>();
    if constexpr (type==WireType::VARINT)
    {
        if constexpr (!is_signed_v<this_type>)
            WriteAsVarint(tgt, obj);
        else if constexpr (is_signed_v<this_type> && sizeof(typename this_type::value_type)<=4)
            WriteAsSignedVarint32(tgt, obj);
        else
            WriteAsSignedVarint64(tgt, obj);
    }
    if constexpr (type==WireType::FIXED32)
        WriteAsFixed32(tgt, obj);
    if constexpr (type==WireType::FIXED64)
        WriteAsFixed64(tgt, obj);
    if constexpr (type==WireType::DELIMITED)
    {
        if constexpr (std::is_same_v<this_type, std::string>)
            WriteDelimitedBytes(tgt, obj);
        else
            tgt << obj;
    }
    return tgt;
}



template<ProtoStruct PS>
inline DataBlock& operator<<(DataBlock& tgt, const PS& obj)
{
    proto_visit(obj, [&tgt](const auto& mbr, auto id)
		{
            using this_type = std::remove_const_t<std::remove_reference_t<decltype(mbr)>>;
            const auto tag = EncodeField<this_type>(id);
            if constexpr (is_non_string_container_v<this_type>) // this needs updating to support packed types
            {
                for (const auto& elem:mbr)
                {
                    using elem_type = std::remove_const_t<std::remove_reference_t<decltype(elem)>>;
                    if (elem != elem_type{})
                    {
                        tgt << tag;
                        if constexpr (!is_proto_struct_v<elem_type>)
                            tgt << elem;
                        else
                        {
                            // message
                            DataBlock tmp;
                            tmp << elem;
                            WriteAsVarint(tgt, (int)std::size(tmp));
                            tgt.insert(tgt.end(), tmp.begin(), tmp.end());
                        }
                    }
                }
            }
            else if constexpr (!is_proto_struct_v<this_type>)
            {
                if (mbr != decltype(mbr){})
                {
                    tgt << tag;
                    tgt << mbr;
                }
            }
            else
            {
                // message
                DataBlock tmp;
                tmp << mbr;
                if (!tmp.empty())
                {
                    tgt << tag;
                    WriteAsVarint(tgt, (int)std::size(tmp));
                    tgt.insert(tgt.end(), tmp.begin(), tmp.end());
                }
            }
		});
    return tgt;
}


// rules for reading from a structure from a ContDataBlock

// rules for writing the schema

template<class PS, class T>
inline std::string type_as_string(T p)
{
    using namespace std::string_literals;
    using RawType = std::remove_const_t<std::remove_reference_t<decltype(std::declval<PS>().*p)>>;
    if (is_non_string_container_v<RawType>)
        return "repeated thing"s;
    if (std::is_same_v<RawType, std::string>)
        return "string"s;
    if (std::is_same_v<RawType, bool>)
        return "bool"s;
    if (std::is_integral_v<RawType> && (sizeof(RawType)<=4))
        return "int32"s;
    if (std::is_integral_v<RawType> && sizeof(RawType)>4)
        return "int64"s;
    if (is_signed_v<RawType> && sizeof(RawType)<=4)
        return "sint32"s;
    if (is_signed_v<RawType> && sizeof(RawType)>4)
        return "sint64"s;
    if (std::is_enum_v<RawType>)
        return typeid(RawType).name();
    return "thing"s;
}

template<EnumeratedType ET>
inline std::string to_schema()
{
    using namespace std::string_literals;
    std::string retval = "enum "s + typeid(ET).name() + "{\n";
    for (const auto e : EnumIterator<ET>{})
        retval += "\t"s + to_string(e) + ((static_cast<ET>((int)(e)+1)==ET::NUM_ENUMS?"\n":",\n"));
    retval += "}";
    return retval;
}


template<ProtoStruct PS>
inline std::string to_schema()
{
    using namespace std::string_literals;
    std::string schema;

    schema += "message "s + typeid(PS).name() + "\n{\n";

    const auto mbrs = PS::get_members();
	const auto call_fn = [&](const auto&...mbr)
	{
		auto new_schema = ""s + (("\t"s+type_as_string<PS>(mbr.pointer) + " " + std::string{mbr.name} + 
                                " = "s + std::to_string(mbr.field_num) + ";\n"s)+ ...);
        schema += new_schema;
	};
    if constexpr (std::tuple_size<decltype(mbrs)>())
    	std::apply(call_fn, mbrs);

    schema += "}\n";
    return schema;
}

// readers

ConstDataBlock operator>>(ConstDataBlock data, uint64_t& tgt)
{
    std::byte next;
    int offset{0};
    tgt = 0;
    do
    {
        data = data>>next;
        tgt |= ((uint64_t)next&0x7F)<<offset;
        offset += 7;
    } while ((bool)(std::byte{0x80}&next));
    return data;
}

template<Numerical T>
ConstDataBlock operator>>(ConstDataBlock data, T& tgt)
{
    uint64_t tmp;
    data = data >> tmp;
    tgt = static_cast<T>(tmp);
    return data;
}

template<EnumType T>
ConstDataBlock operator>>(ConstDataBlock data, T& tgt)
{
    return data >> reinterpret_cast<std::underlying_type_t<T>&>(tgt);    // write as if it was the underlying type
}

template<class T>
ConstDataBlock operator>>(ConstDataBlock data, SignedInt<T>& tgt)
{
    if constexpr (sizeof(T)<=4)
    {
        uint64_t wire_val;
        data = data >> wire_val;
        tgt = (wire_val&1) ? -(T)(wire_val>>1)-1 : (T)(wire_val>>1);
    }
    else
    {
        uint64_t wire_val;
        data = data >> wire_val;
        tgt = (wire_val&1) ? -(T)(wire_val>>1)-1 : (T)(wire_val>>1);
    }
    return data;
}


ConstDataBlock operator>>(ConstDataBlock data, std::string& tgt)
{
    int size;
    data = data >> size;
    tgt.clear();
    for (int i=0; i < size; ++i)
    {
        std::byte c;
        data = data >> c;
        tgt.push_back((char)c);
    }
    return data;
}

template<NonStringContainer C>
ConstDataBlock operator>>(ConstDataBlock data, C& tgt)
{
    using T = typename std::remove_reference_t<C>::value_type; 
    T new_elem{};
    if constexpr (is_proto_struct_v<T>) // embeded object are treated like strings
    {
        int size;
        data = data >> size;
        const auto obj_data = data.first(size);
        const auto unused_data = obj_data >> new_elem;
        assert(unused_data.size()==0);
        tgt.push_back(new_elem);
        return data.subspan(size);
    }
    else
    {
        const auto unused_data = data >> new_elem;
        tgt.push_back(new_elem);
        return unused_data;
    }
}

template<ProtoStruct PS>
ConstDataBlock operator>>(ConstDataBlock data, PS& tgt)
{
    using ReadFn = std::function<ConstDataBlock(ConstDataBlock, WireType)>;
    std::map<int, ReadFn> member_map;
    proto_enumerate(tgt, overloaded{
    [&member_map](auto& tgt, int id, auto )
    {
        member_map[id] = [&tgt](ConstDataBlock data, WireType type){ return data >> tgt;};
    },
    [&member_map]<NonStringContainer T>(T& tgt, int id, auto )
    {
        member_map[id] = [&tgt](ConstDataBlock data, WireType type){ 
            assert/*if*/ (type == WireType::DELIMITED);
            return data >> tgt;;
        };
    },
    [&member_map]<ProtoStruct T>(T& tgt, int id, auto )
    {
        member_map[id] = [&tgt](ConstDataBlock data, WireType type){ 
            assert/*if*/ (type == WireType::DELIMITED);
            int size;
            data = data >> size;
            const auto obj_data = data.first(size);
            const auto unused_data = obj_data >> tgt;
            assert(unused_data.size()==0);
            return data.subspan(size);
        };
    }
    });
    while (!data.empty())
    {
        // is this a byte? or a varint?
        std::byte id_and_type{std::byte{0xFF}};
        data = data >> id_and_type;
        const auto id = static_cast<int>(id_and_type >> 3);
        const WireType type = static_cast<WireType>(id_and_type&std::byte{7});
        const auto found = member_map.find(id);
        if (member_map.end()!=found)
            data = found->second(data, type);
        else
            std::cerr << "cannot find index:" << id << "\n";
            
    }
    return data;
}