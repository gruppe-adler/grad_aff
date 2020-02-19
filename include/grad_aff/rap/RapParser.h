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
    struct hexadecimal : pegtl::if_must< pegtl::istring< '0', 'x' >, numeral_one< pegtl::xdigit, pegtl::one< 'p', 'P' > > > {};
    struct numeral : pegtl::sor< hexadecimal, decimal > {};

    //struct decimalNumber : seq<plus<digit>, opt<keyword<'.'>, plus<digit>>, keyword<';'>> {};// , if_then_else<keyword<'.'>, failure, success >> {};// , if_must<plus<digit>, keyword<'.'>>, must<keyword<';'>> > {};

    struct decimalNumber : seq<decimal> {};


    template< char C > struct string_without : pegtl::star< pegtl::not_one< C, 10, 13 > > {};

    struct quoted_value : pegtl::if_must< pegtl::one< '"' >, string_without< '"' >, pegtl::one< '"' > > {};
    struct single_quoted_value : pegtl::if_must< pegtl::one< '\'' >, string_without< '\'' >, pegtl::one< '\'' > > {};

    struct array_ : if_must< one<'{'>, ws, list_must<sor<decimalNumber, quoted_value, single_quoted_value, array_, ws>, seq<ws, keyword<','>, ws>>, ws, one<'}'>> {};

    struct number : until<keyword<';'>, digit> {};
    struct stringD : keyword<'"'>, until<keyword<';'>, any> {};

    struct tokenKey : seq<identifier, ws, keyword<'='>, ws, sor<decimalNumber, quoted_value, single_quoted_value>, ws, keyword<';'>, ws> {};

    struct token : opt_must<tokenKey, ws> {};

    struct tokenArr : seq<identifier, keyword<'[', ']'>, ws, keyword<'='>, ws, array_, ws, keyword<';'>, ws> {};
       
    struct rapClass : seq<header, ws, opt<star<sor<tokenKey, tokenArr, rapClass>, ws>>, keyword<'}',';'>, ws> {};// , pegtl::opt<inheritance>, pegtl::string<'{'>, pegtl::string<'{', ';' >> {};

    struct topLevel : must<star <ws, sor<rapClass, tokenKey, tokenArr>, ws>, eof> {};

    template< typename Rule >
    struct action
    {};


    template<>
    struct action< identifier >
    {
        /*
        template< typename Input >
        static void apply(const Input& in, std::shared_ptr<RapClass> state)
        {
            auto v = in.string();
            //state->classEntries.push_back()
            std::cout << "Found identifier: " << v << std::endl;

        }
        */

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
        
        template< typename Input >
        static void apply(const Input& in, std::shared_ptr<RapClass>& state)
        {
            auto str = in.string();
            std::cout << str << std::endl;
            state->name = str;
            //state.name = str;
        }

        template< typename Input >
        static void apply(const Input& in, std::shared_ptr<RapValue>& state)
        {
            if (!state) {
                state = std::make_shared<RapValue>();
            }
            auto str = in.string();
            std::cout << str << std::endl;
            state->name = str;
        }
    };

    template<>
        struct action< rapClass >
    {
        /*
        template< typename Input >
        static void apply(const Input& in, std::shared_ptr<RapClass> state)
        {
            auto v = in.string();
            //state->classEntries.push_back()
            std::cout << "Found identifier: " << v << std::endl;

        }
        */

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
    struct action< number >
    {
        template< typename Input >
        static void apply(const Input& in, RapClass state)
        {
            auto v = in.string();
            std::cout << "Found number: " << v << std::endl;
        }
    };

    template<>
    struct action< decimal >
    {
        template< typename Input >
        static void apply(const Input& in, RapClass state)
        {
            auto v = in.string();
            std::cout << "Found decimal: " << v << std::endl;
        }

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
                rapVal->value = std::stol(dec);
                rapVal->type = 1;
                rapVal->subType = 2;
            }
            state[state.size() - 1] = rapVal;
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

        template< typename Input >
        static void apply(const Input& in, std::shared_ptr<RapValue>& state)
        {

            auto v = in.string();
            std::cout << "Found string: " << v << std::endl;
            //state->value = in.string();
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

        template< typename Input >
        static void apply(const Input& in, std::shared_ptr<RapClass>& state)
        {
            auto v = in.string();
            //state->classEntries(Rap)
            std::cout << "Found string: " << v << std::endl;
        }
        template< typename Input >
        static void apply(const Input& in, std::shared_ptr<RapValue>& state)
        {
            auto v = in.string();
            std::cout << "Found string: " << v << std::endl;
            //state->value = in.string();
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