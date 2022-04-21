// GTEST module
#include "protobuf.h"
#include "ReflectionTools.h"
#include <fstream>
#include <filesystem>

namespace testing { 
    namespace internal
    {
        inline bool operator==(ConstDataBlock a, ConstDataBlock b)
        {
            return std::equal(std::begin(a), std::end(a), std::begin(b), std::end(b));
        }
        std::ostream& operator<<(std::ostream& os, const std::byte val)
        {
            os << (int)val; 
            return os;
        }
        std::ostream& operator<<(std::ostream& os, ConstDataBlock val)
        {
            os << "[" << val.size() << "]{" << std::hex;
            bool first = true;
            for (const auto v:val)
            {
                os << (first?"":",")<<(int)v;
                first = false;
            }
            os << "}";
            return os;
        }
    }
}

DataBlock ReadFile(std::string name)
{
    if (!std::filesystem::exists(name))
    {
        std::cerr << "Cannot find file \"" << name << "\"\n";
        return {};
    }
    if (!std::filesystem::is_regular_file(name))
    {
        std::cerr << "File is not a regular file (is it a directory?): \"" << name << "\"\n";
        return {};
    }
    const auto size = std::filesystem::file_size(name);
    std::ifstream file(name, std::ios::binary);
    DataBlock retval(size);
    file.read((char*)retval.data(), retval.size());
    return retval;
}

#include <gtest/gtest.h>

TEST(ProtoBuf, BytePush)
{
    DataBlock tgt;
    std::byte test_bytes[] = {std::byte{0xFF}, std::byte{0x00}, std::byte{0x40}, std::byte{0x20}, 
                                std::byte{0x10}, std::byte{0x08}, std::byte{0x04}, std::byte{0x02}, std::byte{0x01} };
    for(const auto v: test_bytes)
        tgt << v;
    EXPECT_EQ(ConstDataBlock{test_bytes}, as_const(tgt));
}

TEST(ProtoBuf, VarInt32)
{
    std::byte test_0_bytes[] = {std::byte{0x00}};
    std::byte test_1_bytes[] = {std::byte{0x01}};
    std::byte test_150_bytes[] = {std::byte{0x96}, std::byte{0x01} };
    std::byte test_300_bytes[] = {std::byte{0xAC}, std::byte{0x02} };
    std::byte test_neg1_bytes[] = {std::byte{0xFF}, std::byte{0xFF} , std::byte{0xFF}, std::byte{0xFF}, std::byte{0xFF},
                                    std::byte{0xFF}, std::byte{0xFF} , std::byte{0xFF}, std::byte{0xFF}, std::byte{0x01}};
    std::vector<std::pair<ConstDataBlock, int>> test_pairs = { {test_0_bytes, 0}, {test_1_bytes, 1},
                                                            {test_150_bytes, 150}, {test_neg1_bytes, -1}, {test_300_bytes, 300} };
    for (const auto& pair : test_pairs)
    {
        DataBlock tgt;
        WriteAsVarint(tgt, pair.second);
        EXPECT_EQ(ConstDataBlock{pair.first}, as_const(tgt));
        auto src = as_const(tgt);
        int v;
        const auto src_res = src >> v;
        EXPECT_EQ(src_res.size(), 0);
        EXPECT_EQ(v, pair.second);
    }
}

template<class Orig, class Pairs, class Fn>
void TestVarIntFn( const Pairs test_pairs, const Fn&& fn)
{
    for (const auto& pair : test_pairs)
    {
        using namespace testing::internal;
        DataBlock tgt;
        fn(tgt, pair.second);
        EXPECT_EQ(ConstDataBlock{pair.first}, as_const(tgt)) << "encoding:" << std::dec << pair.second << "(" << std::hex << pair.second << ")";
        const auto in_data = as_const(tgt); 
        Orig test;
        const auto in_data_res = in_data >> test;
        EXPECT_EQ(in_data_res.size(), 0);   
        EXPECT_EQ(test, pair.second)  << "encoding:" << std::dec << pair.second << "(" << std::hex << pair.second << ")" << " vs (" << test << ")";
    }
}

TEST(ProtoBuf, SignedVarInt32)
{
    std::byte test_0_bytes[] = {std::byte{0x00}};
    std::byte test_neg1_bytes[] = {std::byte{0x01}};
    std::byte test_1_bytes[] = {std::byte{0x02}};
    std::byte test_negmax_bytes[] = {std::byte{0xFF}, std::byte{0xFF} , std::byte{0xFF}, std::byte{0xFF}, std::byte{0x0F}};
    std::byte test_max_bytes[] = {std::byte{0xFE}, std::byte{0xFF} , std::byte{0xFF}, std::byte{0xFF},  std::byte{0x0F}};
    std::byte test_150_bytes[] = {std::byte{0xAC}, std::byte{0x02} };
    std::vector<std::pair<ConstDataBlock, int>> test_pairs = { {test_0_bytes, 0}, {test_neg1_bytes, -1}, {test_1_bytes, 1},
                                     {test_negmax_bytes, -2147483648}, {test_max_bytes, 2147483647}, {test_150_bytes, 150 }};

    TestVarIntFn<SignedInt<int32_t>>(test_pairs, [](auto& tgt, const auto val){ return WriteAsSignedVarint32(tgt, val);});
}

TEST(ProtoBuf, SignedVarInt64)
{
    // 32-bit should be the same
    std::byte test_0_bytes[] = {std::byte{0x00}};
    std::byte test_neg1_bytes[] = {std::byte{0x01}};
    std::byte test_1_bytes[] = {std::byte{0x02}};
    std::byte test_negmax_bytes[] = {std::byte{0xFF}, std::byte{0xFF} , std::byte{0xFF}, std::byte{0xFF}, std::byte{0x0F}};
    std::byte test_max_bytes[] = {std::byte{0xFE}, std::byte{0xFF} , std::byte{0xFF}, std::byte{0xFF},  std::byte{0x0F}};
    std::byte test_150_bytes[] = {std::byte{0xAC}, std::byte{0x02} };
    std::vector<std::pair<ConstDataBlock, int64_t>> test_pairs = { 
                                    {test_0_bytes, 0}, {test_neg1_bytes, -1}, {test_1_bytes, 1},
                                     {test_negmax_bytes, -2147483648}
                                     , {test_max_bytes, 2147483647}, {test_150_bytes, 150 }
        };

    TestVarIntFn<SignedInt<int64_t>>(test_pairs, [](auto& tgt, const auto val){ return WriteAsSignedVarint64(tgt, val);});

    std::byte test_negbig_bytes[] = {std::byte{0xFD}, std::byte{0xFF}, std::byte{0xFF}, std::byte{0xFF}, std::byte{0xFF},
                                     std::byte{0xFF}, std::byte{0xFF}, std::byte{0xFF}, std::byte{0x1F} };

    std::vector<std::pair<ConstDataBlock, int64_t>> test64_pairs = { 
        {test_negbig_bytes, -0xFFFFFFFFFFFFFFF} 
    }; 
    TestVarIntFn<SignedInt<int64_t>>(test64_pairs, [](auto& tgt, const auto val){ return WriteAsSignedVarint64(tgt, val);});
}

TEST(ProtoBuf, HelloWorld)
{
    struct HelloRequest {
    std::string name;
    int32_t var32;
    int64_t var64;
    SignedInt<int32_t> s32;
    SignedInt<int64_t> s64;
    static constexpr auto get_members() {
        return std::make_tuple(
                PROTODECL(HelloRequest, 1, name),
                PROTODECL(HelloRequest, 2, var32),
                PROTODECL(HelloRequest, 3, var64),
                PROTODECL(HelloRequest, 4, s32),
                PROTODECL(HelloRequest, 5, s64)
        );
        }
    };
    HelloRequest test{.name="world", .var32=150, .var64=150, .s32=150, .s64=-0xFFFFFFFFFFFFFFF};
    DataBlock tgt;
    tgt << test;
    std::byte wireshark_snoop[] = {
        std::byte{0x0a},
        std::byte{0x05},
        std::byte{0x77},
        std::byte{0x6f},
        std::byte{0x72},
        std::byte{0x6c},
        std::byte{0x64},
        std::byte{0x10},
        std::byte{0x96},
        std::byte{0x01},
        std::byte{0x18},
        std::byte{0x96},
        std::byte{0x01},
        std::byte{0x20},
        std::byte{0xac},
        std::byte{0x02},
        std::byte{0x28},
        std::byte{0xfd},
        std::byte{0xff},
        std::byte{0xff},
        std::byte{0xff},
        std::byte{0xff},
        std::byte{0xff},
        std::byte{0xff},
        std::byte{0xff},
        std::byte{0x1f},
    };
    EXPECT_EQ(wireshark_snoop, tgt);
}

TEST(ProtoBuf, Compound)
{
    struct Surname {
    std::string name;
    static constexpr auto get_members() {
        return std::make_tuple(
                PROTODECL(Surname, 1, name)
        );
        }
    };
    struct HelloRequest {
    std::string name;
    Surname     surname;
    static constexpr auto get_members() {
        return std::make_tuple(
                PROTODECL(HelloRequest, 1, name),
                PROTODECL(HelloRequest, 2, surname)
        );
        }
    };
    HelloRequest test{.name="Mike", .surname={"Crotch"}};
    DataBlock tgt;
    tgt << test;
    char wireshark_snoop[] = "\x0a\x04\x4d\x69\x6b\x65\x12\x08\x0a\x06\x43\x72\x6f\x74\x63\x68";
    EXPECT_EQ(std::size(wireshark_snoop)-1, std::size(tgt));
    const auto as_span = std::span{wireshark_snoop}; 
    const auto drop_terminator = as_span.first(as_span.size()-1);
    EXPECT_EQ(std::as_bytes(drop_terminator), tgt);
}

TEST(ProtoBuf, Repeated)
{
    struct Surname {
        std::string name;
        static constexpr auto get_members() {
            return std::make_tuple(
                    PROTODECL(Surname, 1, name)
            );
            }
    };
    struct HelloRequest {
        std::string name;
        std::vector<Surname>     surname;
        static constexpr auto get_members() {
            return std::make_tuple(
                    PROTODECL(HelloRequest, 1, name),
                    PROTODECL(HelloRequest, 2, surname)
            );
            }
    };
    static_assert(is_proto_struct_v<Surname>);

    HelloRequest test{.name="Mike", .surname={{{"bloaty"}, {"mcbloatface"}}}};
    DataBlock tgt;
    tgt << test;
    char wireshark_snoop[] =    "\x0a\x04\x4d\x69\x6b\x65\x12\x08\x0a\x06\x62\x6c\x6f\x61\x74\x79" \
                                "\x12\x0d\x0a\x0b\x6d\x63\x62\x6c\x6f\x61\x74\x66\x61\x63\x65";
    EXPECT_EQ(std::size(wireshark_snoop)-1, std::size(tgt));
    const auto as_span = std::span{wireshark_snoop}; 
    const auto drop_terminator = as_span.first(as_span.size()-1);
    EXPECT_EQ(std::as_bytes(drop_terminator), tgt);
}

TEST(ProtoBuf, RepeatedEmbeded)
{
    struct Nickname
    {
        std::string name;
        static constexpr auto get_members() {
            return std::make_tuple(
                    PROTODECL(Nickname, 1, name)
            );
        }
    };
    struct Surname {
        std::string name;
        std::vector<Nickname> moniker;
        static constexpr auto get_members() {
            return std::make_tuple(
                    PROTODECL(Surname, 1, name),
                    PROTODECL(Surname, 2, moniker)
            );
        }
    };
    struct HelloRequest {
        std::vector<std::string> name;
        std::vector<Surname>     surname;
        static constexpr auto get_members() {
            return std::make_tuple(
                    PROTODECL(HelloRequest, 1, name),
                    PROTODECL(HelloRequest, 2, surname)
            );
            }
    };
    HelloRequest test                   {
                                            .name={"Mike", "Ike"},
                                            .surname={
                                                    {.name="bloaty", .moniker{{"bloat-master"}, {"code-killer"}}}, 
                                                    {.name="mcbloatface"}
                                            }
                                        };

    DataBlock tgt;
    tgt << test;
    char wireshark_snoop[] =    "\x0a\x04\x4d\x69\x6b\x65\x0a\x03\x49\x6b\x65\x12\x27\x0a\x06\x62" \
                                "\x6c\x6f\x61\x74\x79\x12\x0e\x0a\x0c\x62\x6c\x6f\x61\x74\x2d\x6d" \
                                "\x61\x73\x74\x65\x72\x12\x0d\x0a\x0b\x63\x6f\x64\x65\x2d\x6b\x69" \
                                "\x6c\x6c\x65\x72\x12\x0d\x0a\x0b\x6d\x63\x62\x6c\x6f\x61\x74\x66" \
                                "\x61\x63\x65";
    EXPECT_EQ(std::size(wireshark_snoop)-1, std::size(tgt));
    const auto as_span = std::span{wireshark_snoop}; 
    const auto drop_terminator = as_span.first(as_span.size()-1);
    EXPECT_EQ(std::as_bytes(drop_terminator), tgt);

    const auto src = as_const(tgt);
    HelloRequest read_tgt; 
    const auto read_res = src >> read_tgt; 
    EXPECT_EQ(read_res.size(), 0);
    EXPECT_EQ(test, read_tgt);
}


struct Empty
{
    static constexpr auto get_members() {
        return std::make_tuple(
        );
    }
};

TEST(ProtoBufSchema, Empty)
{
    std::string schema = to_schema<Empty>();
    const auto expected = std::string{"message 5Empty\n{\n}\n"};
    EXPECT_EQ(expected, schema);
}

TEST(ProtoBufSchema, Simple)
{
    struct HelloRequest {
        std::string name;
        bool on_off;
        int32_t var32;
        int64_t var64;
        SignedInt<int32_t> s32;
        SignedInt<int64_t> s64;
        enum class PhoneType {
            MOBILE,
            HOME,
            WORK,
            NUM_ENUMS
        } phone_type;
        static std::string to_string(PhoneType v) {
            switch(v){
                case PhoneType::MOBILE: return "MOBILE";
                case PhoneType::HOME: return "HOME";
                case PhoneType::WORK: return "WORK";
                default:
                    return "Unknown";
            }
        }
        static constexpr auto get_members() {
            return std::make_tuple(
                    PROTODECL(HelloRequest, 1, name),
                    PROTODECL(HelloRequest, 2, on_off),
                    PROTODECL(HelloRequest, 3, var32),
                    PROTODECL(HelloRequest, 4, var64),
                    PROTODECL(HelloRequest, 5, s32),
                    PROTODECL(HelloRequest, 6, s64),
                    PROTODECL(HelloRequest, 6, phone_type)
            );
        }
    }; 
    std::string schema = to_schema<HelloRequest>();
    std::cout << schema + "\n";
}


TEST(ProtoBufSchema, SkipEmptyFields)
{
    struct SubObject{
        int a{0};
        static constexpr auto get_members() {
            return std::make_tuple(
                    PROTODECL(SubObject, 1, a)
            );
        }
    };
    struct HelloRequest {
        std::string name;
        bool on_off;
        int32_t var32;
        int64_t var64;
        SignedInt<int32_t> s32;
        SignedInt<int64_t> s64;
        enum class PhoneType {
            MOBILE,
            HOME,
            WORK,
            NUM_ENUMS
        } phone_type;
        SubObject sub;
        static constexpr auto get_members() {
            return std::make_tuple(
                    PROTODECL(HelloRequest, 1, name),
                    PROTODECL(HelloRequest, 2, on_off),
                    PROTODECL(HelloRequest, 3, var32),
                    PROTODECL(HelloRequest, 4, var64),
                    PROTODECL(HelloRequest, 5, s32),
                    PROTODECL(HelloRequest, 6, s64),
                    PROTODECL(HelloRequest, 7, phone_type),
                    PROTODECL(HelloRequest, 8, sub)
            );
        }
    };
    {
        HelloRequest test{};
        DataBlock tgt;
        tgt << test;
        EXPECT_EQ(tgt.size(),0);
    }
    {
        HelloRequest test{.s32=1};
        DataBlock tgt;
        tgt << test;
        EXPECT_EQ(tgt.size(),2);
    }
}


enum class PhoneType {
    MOBILE,
    HOME,
    WORK,
    NUM_ENUMS
};

static std::string to_string(PhoneType v) {
    switch(v){
        case PhoneType::MOBILE: return "MOBILE";
        case PhoneType::HOME: return "HOME";
        case PhoneType::WORK: return "WORK";
        default:
            return "Unknown";
    }
}

TEST(ProtoBufSchema, Enums)
{
    std::string schema = to_schema<PhoneType>();
    const auto expected = "enum 9PhoneType{\n	MOBILE,\n	HOME,\n	WORK\n}";
    EXPECT_EQ(schema, expected);
}

TEST(ProtoBufRead, string)
{
    std::byte wireshark_snoop[] = {
        std::byte{0x05},
        std::byte{0x77},
        std::byte{0x6f},
        std::byte{0x72},
        std::byte{0x6c},
        std::byte{0x64}
    };
    std::string test;
    ConstDataBlock input{wireshark_snoop};
    const auto remaining = input >> test;
    EXPECT_EQ(remaining.size(), 0);
    EXPECT_EQ(test, "world");
}


TEST(ProtoBuf, Read)
{
    using namespace std::string_literals;
    struct HelloRequest {
        std::string name;
        int32_t var32;
        int64_t var64;
        SignedInt<int32_t> s32;
        SignedInt<int64_t> s64;
        static constexpr auto get_members() {
            return std::make_tuple(
                    PROTODECL(HelloRequest, 1, name),
                    PROTODECL(HelloRequest, 2, var32),
                    PROTODECL(HelloRequest, 3, var64),
                    PROTODECL(HelloRequest, 4, s32),
                    PROTODECL(HelloRequest, 5, s64)
            );
        }
    };
    std::byte wireshark_snoop[] = {
        std::byte{0x0a},
        std::byte{0x05},
        std::byte{0x77},
        std::byte{0x6f},
        std::byte{0x72},
        std::byte{0x6c},
        std::byte{0x64},
        std::byte{0x10},
        std::byte{0x96},
        std::byte{0x01},
        std::byte{0x18},
        std::byte{0x96},
        std::byte{0x01},
        std::byte{0x20},
        std::byte{0xac},
        std::byte{0x02},
        std::byte{0x28},
        std::byte{0xfd},
        std::byte{0xff},
        std::byte{0xff},
        std::byte{0xff},
        std::byte{0xff},
        std::byte{0xff},
        std::byte{0xff},
        std::byte{0xff},
        std::byte{0x1f},
    };
    HelloRequest test{};
    ConstDataBlock input{wireshark_snoop};
    input >> test;
    HelloRequest expected{.name="world", .var32=150, .var64=150, .s32=150, .s64=-0xFFFFFFFFFFFFFFF};
    EXPECT_EQ(test, expected);
}


// This is a real world example from the protobuf examples 
TEST(ProtoBuf, AddressBook)
{
    // Types required
    using std::string;
    struct Timestamp {
        int64_t seconds;
        int32_t nanos;
         static constexpr auto get_members() {
            return std::make_tuple(
                    PROTODECL(Timestamp, 1, seconds),
                    PROTODECL(Timestamp, 2, nanos)
            );
        }
    };

    struct Person {
        string name;
        int32_t id;
        string email;

        enum class PhoneType {
            MOBILE,
            HOME,
            WORK,
            NUM_ENUMS
        } phone_type;

        static std::string to_string(PhoneType v) {
            switch(v){
                case PhoneType::MOBILE: return "MOBILE";
                case PhoneType::HOME: return "HOME";
                case PhoneType::WORK: return "WORK";
                default:
                    return "Unknown";
            }
        }        

        struct PhoneNumber {
            string number;
            PhoneType type;
            static constexpr auto get_members() {
                return std::make_tuple(
                        PROTODECL(PhoneNumber, 1, number),
                        PROTODECL(PhoneNumber, 2, type)
                );
            }
        };

        std::vector<PhoneNumber> phones;

        Timestamp last_updated;
        static constexpr auto get_members() {
            return std::make_tuple(
                    PROTODECL(Person, 1, name),
                    PROTODECL(Person, 2, id),
                    PROTODECL(Person, 3, email),
                    PROTODECL(Person, 4, phones),
                    PROTODECL(Person, 5, last_updated)
            );
        }
    };

    // Our address book file is just one of these.
    struct AddressBook {
        std::vector<Person> people;
        static constexpr auto get_members() {
            return std::make_tuple(
                    PROTODECL(AddressBook, 1, people)
            );
        }
    };
    const auto data = ReadFile("../address.book.data");
    AddressBook book{};
    data >> book;
    ASSERT_EQ(book.people.size(), 1);
    EXPECT_EQ(book.people[0].email, "honk@frumpy.com");
    EXPECT_EQ(book.people[0].name, "Honk");
    ASSERT_EQ(book.people[0].phones.size(), 1);
    EXPECT_EQ(book.people[0].phones[0].number, "0123456789");
    EXPECT_EQ(book.people[0].phones[0].type, Person::PhoneType::MOBILE);
}