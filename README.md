# TinyPB
A hobby code implimentation of Google ProtoBuffers - natively in (real) C++, not C++ pretending to be C#

Google ProtoBuffers is highly annoying, in that it boasts (very loudly) about how effecient it is. However, viewed from a modern C++ viewpoint it is a bloated and badly structured. It was obviously written by authors that only dabble in C++.

However, it is also not hard to see why it has proven popular and gained traction.
- It is much more compact than similar text-based protocols for exchanging data between languages.
- It supports a good number of languages.
- It has good tool support

The main problems stem from the fact that it supports compiled languages as almost an afterthought. It wants to be a runtime system, because that is what most of the languages it supports are. It also fundementally misunderstands reflection in C++ (it is not alone in this, so does Qt/Unreal/Circle and many others, seriously why has evryone got this so wrong!). If you want to support strongly typed languages, you must do so by using strong-typing. Once you use "any" types, you can never go back. Don't do it!

This project is based on the reflection system I wrote for the NetworkingExamples project. It uses types (structs/classes) to describe the schema. Those types use conventions to make use of generic algorithms (templates) to generate input/output code. It can even generate that code that outputs the schema in proto-format.

This is what I mean by natively C++.

For example... this is what the Address Book example from protobuf looks like. From this you can build "<<" and ">>" operators that load/save in compatible binary formats. One day (soon I hope), the standards commitee will finish the reflection system in C++ and this code will be sort-of compatible (or at least will use the same way of thinking).

```
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
```
As it is template-based, this may impact your build times, but it is also  
a) Single pass (no protoc required)  
b) header-only  
It requires C++20, but that is mostly because I wanted to try using concepts. It can easily be ported to C++17, or older (but it gets messy before C++17)


# Testing and support

This is "as-is" and definietely WIP. There is a google-test based test for many things, but still no support for  
 a) repeated [packed = true]  
 b) only supports proto3  
 c) the schema output is only PoC, and needs a think/rethink  

Given the rate of bugs I have found, I would suspect even the things it supports are fragile. (e.g. I have not tested containers other than vectors, etc).
That said, if you are a C++ programmer, then the problems are in C++, and therefore, you can fix them, so why not try?

# NOTE:
It looks like I need to backport it to C++17 to get the ci to work, as C++20==GNUC++2a, which is ancient
If you have a compiler from this decade, it should build the tests using cmake.

# To try it
Path the "include" directory. Include the "protobuf.h" file. Define your message in C++ struct(s), see "protobuf.cpp" for examples.
Then pipe you structure to/from a byte array.
I might extend with a networking layer, but *not* RPC. RPC sucks. That's why everyone uses async-RPC, but async-RPC is just a confused metaphor. It's like having an async-mutex! Race-conditions, anyone?
