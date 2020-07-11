#pragma once


#include <tao/pegtl.hpp>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/compare.hpp>
#include <boost/algorithm/string/find.hpp>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "Class.h"
#include "ClassEntry.h"

namespace pegtl = tao::pegtl;
namespace ba = boost::algorithm;

using namespace pegtl;

namespace grad_aff::RapParser2
{
    struct ws : star<space> {};

    struct classPrefix : pegtl::keyword<'c', 'l', 'a', 's', 's'> {};
        
    struct name : identifier {};

    struct inheritance : opt_must< keyword<':'>, ws, identifier> {};

    struct header : seq< classPrefix, ws, identifier, ws, inheritance, ws, keyword<'{'>> {};

    template< typename E >
    struct exponent : pegtl::opt_must< E, pegtl::opt< pegtl::one< '+', '-' > >, pegtl::plus< pegtl::digit > > {};

    template< typename D, typename E >
    struct numeral_three : pegtl::seq< pegtl::if_must< pegtl::one< '.' >, pegtl::plus< D > >, exponent< E > > {};
    template< typename D, typename E >
    struct numeral_two : pegtl::seq< pegtl::plus< D >, pegtl::opt< pegtl::one< '.' >, pegtl::star< D > >, exponent< E > > {};
    template< typename D, typename E >
    struct numeral_one : pegtl::sor< numeral_two< D, E >, numeral_three< D, E > > {};

    struct decimal : numeral_one< pegtl::digit, pegtl::one< 'e', 'E' > > {};

    //struct decimalNumber : seq<plus<digit>, opt<keyword<'.'>, plus<digit>>, keyword<';'>> {};// , if_then_else<keyword<'.'>, failure, success >> {};// , if_must<plus<digit>, keyword<'.'>>, must<keyword<';'>> > {};

    struct decimalNumber : seq<decimal> {};

    //struct dec2 : seq < opt<one< '+', '-' >>, plus<digit>, if_must<one< '.' >, plus<digit>> > {};
    struct dec2 : seq <opt<one< '+', '-' >>, plus<digit>, opt<if_must<one< '.' >, plus<digit>>> > {};

    struct decArr : dec2 {};

    template< char C > struct string_without : pegtl::star< pegtl::not_one< C, 10, 13 > > {};

    struct quoted_value : if_must< one< '"' >, string_without< '"' >, one< '"' > > {};
    struct single_quoted_value : if_must< one< '\'' >, string_without< '\'' >, one< '\'' > > {};

    struct array_ : if_must< one<'{'>, ws, opt<list_must<sor<dec2, quoted_value, single_quoted_value, array_>, seq<ws, one<','>, ws>>>, ws, one<'}'>> {};

    struct number : until<keyword<';'>, digit> {};
    struct stringD : keyword<'"'>, until<keyword<';'>, any> {};

    struct tokenKey : seq<identifier, ws, one<'='>, ws, sor<dec2, quoted_value, single_quoted_value>, ws, keyword<';'>, ws> {};

    struct tokenArr : seq<identifier, keyword<'[', ']'>, ws, one<'='>, ws, array_, ws, keyword<';'>, ws> {};
       
    struct rapClass : seq<header, ws, opt<star<sor<tokenKey, tokenArr, rapClass>, ws>>, keyword<'}',';'>, ws> {};// , pegtl::opt<inheritance>, pegtl::string<'{'>, pegtl::string<'{', ';' >> {};

    struct topLevel : must<star <ws, sor<rapClass, tokenKey, tokenArr>, ws>, eof> {};

    template< typename Rule >
    struct action
    {};

    template<>
    struct action< array_ >
    {
        template< typename Input >
        static void apply(const Input& in, std::vector< std::shared_ptr<ClassEntry>>& state)
        {
            std::string str = in.string();

            state.pop_back();
            state.back()->type = 2;

            auto lastEntryName = state.back()->name;
            auto rapArr = std::make_shared<RapArray>();
            rapArr->name = lastEntryName;
            rapArr->type = 2;
            std::string dec = in.string();
            rapArr->arrayElements.clear();
            rapArr->arrayElements.push_back(str);
            // TODO:
            //state.push_back(rapArr);
        }
    };

    template<>
    struct action< identifier >
    {
        template< typename Input >
        static void apply(const Input& in, std::vector< std::shared_ptr<ClassEntry>>& state)
        {
            std::string str = in.string();

            if (boost::equals(str, "class")) {
                return;
            }

            std::cout << str << std::endl;
            auto rc = std::make_shared<ClassEntry>();
            rc->name = str;
            state.push_back(rc);
            //state.name = str;
        }        
    };

    template<>
    struct action< rapClass >
    {

        template< typename Input >
        static void apply(const Input& in, std::vector< std::shared_ptr<ClassEntry>>& state)
        {
            auto str = in.string();
            std::cout << str << std::endl;
            auto rc = std::make_shared<ClassEntry>();
            rc->name = "null";
            state.push_back(std::shared_ptr<ClassEntry>(nullptr));
            //state.name = str;
        }


    };

    template<>
    struct action< inheritance >
    {
        template< typename Input >
        static void apply(const Input& in, std::vector< std::shared_ptr<ClassEntry>>& state)
        {
            auto v = in.string();

            if (v.empty()) {
                return;
            }

            auto inheritedName = state.back()->name;
            state.pop_back();
            auto className = state.back()->name;
            state.pop_back();

            auto rc = std::make_shared<RapClass>();
            rc->name = className;
            rc->inheritedClassname = inheritedName;
            state.push_back(rc);
            

            std::cout << "Found inheritance: " << v << std::endl;
        }
    };

    template<>
    struct action< decimal >
    {
        template< typename Input >
        static void apply(const Input& in, std::vector< std::shared_ptr<ClassEntry>>& state)
        {
            auto lastEntryName = std::static_pointer_cast<RapValue>(state.back())->name;
            auto rapVal = std::make_shared<RapValue>();
            rapVal->name = lastEntryName;
            std::string dec = in.string();

            if (dec.find(".") != dec.npos) {
                rapVal->value = std::stof(dec);
                rapVal->type = 1;
                rapVal->subType = 1;
            }
            else {
                rapVal->value = std::stoi(dec);
                rapVal->type = 1;
                rapVal->subType = 2;
            }
            state[state.size() - 1] = rapVal;
        }

    };

    template<>
    struct action< dec2 >
    {
        template< typename Input >
        static void apply(const Input& in, std::vector< std::shared_ptr<ClassEntry>>& state)
        {
            auto lastEntryName = std::static_pointer_cast<RapValue>(state.back())->name;
            auto rapVal = std::make_shared<RapValue>();
            rapVal->name = lastEntryName;
            std::string dec = in.string();

            if (dec.find(".") != dec.npos) {
                rapVal->value = std::stof(dec);
                rapVal->type = 1;
                rapVal->subType = 1;
            }
            else {
                rapVal->value = std::stoi(dec);
                rapVal->type = 1;
                rapVal->subType = 2;
            }
            state[state.size() - 1] = rapVal;
        }

    };

    template<>
    struct action< decArr >
    {
        template< typename Input >
        static void apply(const Input& in, std::vector< std::shared_ptr<ClassEntry>>& state)
        {

            if (state.back()->type != 2) {
                auto lastEntryName = state.back()->name;
                state.pop_back();
                auto rapVal = std::make_shared<RapArray>();
                rapVal->name = lastEntryName;
                rapVal->type = 2;
                state.push_back(rapVal);
            }

            auto arr = std::static_pointer_cast<RapArray>(state.back());

            std::variant<std::string, float_t, int32_t, RapArray> value;
            std::string dec = in.string();

            if (dec.find(".") != dec.npos) {
                value = std::stof(dec);
            }
            else {
                value = std::stoi(dec);
            }
            arr->arrayElements.push_back(value);
        }

    };

    template<>
    struct action< string_without<'"'> >
    {
        template< typename Input >
        static void apply(const Input& in, std::vector< std::shared_ptr<ClassEntry>>& state)
        {
            auto lastEntryName = std::static_pointer_cast<RapValue>(state.back())->name;
            auto rapVal = std::make_shared<RapValue>();
            rapVal->name = lastEntryName;
            rapVal->value = in.string();
            rapVal->type = 1;
            rapVal->subType = 0;
            state[state.size() - 1] = rapVal;
        }
    };

    template<>
    struct action< string_without<'\''> >
    {
        template< typename Input >
        static void apply(const Input& in, std::vector< std::shared_ptr<ClassEntry>>& state)
        {
            auto lastEntryName = std::static_pointer_cast<RapValue>(state.back())->name;
            auto rapVal = std::make_shared<RapValue>();
            rapVal->name = lastEntryName;
            rapVal->value = in.string();
            state[state.size() - 1] = rapVal;
        }
    };
    /*
    template<>
    struct action< tokenKey >
        : tao::pegtl::change_states<std::shared_ptr<RapValue>>
    {
        template< typename Input >
        static void apply(const Input& in, std::shared_ptr<RapValue>& newRapValue, std::shared_ptr<RapClass>& rapClass)
        {
            newRapValue = std::make_shared<RapValue>();
            
            rapClass.classEntries.push_back(newRapValue);
        }

        template< typename Input >
        static void success(const Input& in, std::shared_ptr<RapValue>& newRapValue, std::shared_ptr<RapClass>& rapClass)
        {
            newRapValue = std::make_shared<RapValue>();

            rapClass->classEntries.push_back(newRapValue);
            //newRapValue = std::make_shared<RapValue>();
            //rapClass.classEntries.push_back(newRapValue);
        }
    };
    */

}

namespace grad_aff {
    class RapParser {
    public:
        RapParser();
        void parseConfig(std::string filename);
    };
}