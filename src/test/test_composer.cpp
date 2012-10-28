/*

Formatter strings:
formatter string consist of at least three kinds of elements

1. Literal elements
Literal elements are, as the name implies, chunks of text that are sent to the connected
interace as they are appear in the format string. Exeptions of this are non printable ascii chars
which can be inserted in literal string elements using the hash syntax. Consider the example below:
#2;teststring#3;
This literal will produce the same as the C literal "\x02teststring\x03".
Every legal ascii char up to 127 is allowed. There is neiter unicode support now nor support for 
unsigned char values up to 255.

2. Datefields and methods.
A field call will introduce a field object in the list of text chunks. If no further method is chained
after the field name, the object is just in the list and if a new data set input comes in, the 
field object will look up the field in the data set an will print the unformatted value to the output
stream.
EXAMPLE

test::input inp("testinput").add("name", "Isaac Asimov");
composer::message msg("testinput", "Favorite SciFi writer: $(name)");
std::cout << msg.execute(inp) << std::cout;

will print 
Favorite SciFi writer: Isaac Asimov

However, you can append, "chained" method calls to the aforementioned field descriptor, causing
arbitrary actions to execute on the field. Consider this:


composer::message msg("testinput", "Favorite SciFi writer: $(name).mid(6, 6)");

would produce

Favorite SciFi writer: Asimov

Supported field methods:
add(NUMERIC)
will add a fixed numeric amount to the field.
The value of add must be numeric (float or int) and the
field itself must be numeric as will. This will throw at format string compile time if the field cannot
be validated as numeric.

sub(NUMERIC)
like add but does substraction. The resulting value can be negative. See the fmt function on how to
format numeric values.

mul(NUMERIC)
like add but multiplies the value

div(NUMERIC)
like add but diviedes the value. Throws on division by zero.

mid(offset,length)
cuts a substring beginning at numeric 
offset "offset" with the length "length". Example $("teststring").mid(0, 4) == "test".

replace("original", "replacement")
Works on strings, and will handle numeric expressions like strings.
Exmaple $("only the original will cost you 12 Dollar")
    .replace("original", "replacement")
    .replace("12", "6") == "only the replacement will cost you 6 Dollar" ...

find(dictionary,field)
Will search in the list of added dictionaries for the dictionary named like
the argument "dictionary" and will pass the value of the field as search parameter.
On a successful call if will fetch the result field named by the second parameter
"field" and will pass it's value as the result.
Throws if there is no dictionary with that name and throws as well if the search term
passed, retreives no result set and throws if there is no such field name as in
the second argument "field".
To learn more about dictionaries and events, see the DICTIONARY section of this manual.

fmt(FORMAT)
will format the field according to the passed format string. Please consult the man page
of a standard C compiler "printf" function to learn on how to create a format string. 

General hints:
Note that chainable methods can be connected in in unlimited number. Check out the
example below, we will go through it step by step:

EXAMPLE

#2;EXTRACT:$(fieldname).mid(2, 4).find(items, on_stock).add(2).fmt(%02.2d)#3;

lets pretend fieldname is an 12 digits identifier in which different parts have different
meanings for a rather complicated article database.
Let's further pretend that the figures 2 up to 6 are ment as a primary key in this database.
So, if the execute function is called on the message object, we will extract this key from
the string at first and then we will pass this key to a SQL query like
SELECT on_stock, name, price FROM items_to_order WHERE id=:id.
We will pass back the on_stock field, which is a numeric value. But we don't want to pass the
exact amount but the amount +2 items, because the management decided in their latest meeting
always to order two extra items "just in case".
Last but not least: The caotic storage management software of the high rack warehouse only 
accepts values exactly 4 digits width padded by leading zeroes.
All the actions, mentioned above are executed by this example. Just read all the chained functions
calls carefully from left to right and you will get a feeling on how to use this in your own
application.


3. generators
generators ar functions that create values without any base on fields from an input record.
Right now there are three such generators, that can be combined in turn with 
special chainable functions.
Those are:

1. now()
The now function creates a date value from the current millisecond.
functions
add(num, unit)
sub(num,  unit)
dtfmt(format)

2. checksum
Creates CRC checksum as used by must RS232 protocols. The current result string to the
point where this function is called, is considered.

3. counter(name)
Creates a counter object 
functions
inc(value)
dec(value)
resetOn(value)


EXAMPLE:
#2;TR|TN$(fieldname).find(dictionary1,column1).mid(2,4).fmt(%4.4d)|T2$now().add(1, d).dfmt(%H:%M:%S)#3;

 * */
#include <boost/test/unit_test.hpp>
#include "../composer.hpp"

#include <boost/unordered_map.hpp>

BOOST_AUTO_TEST_SUITE( composer_test )

struct dummy_input : public composer::input
{

    std::string name_;
    dummy_input( const std::string &s ):
        name_(s)
    {
    
    }

    const std::string& name() const
    {
        return name_;
    }
    
    boost::unordered_map<std::string, std::string> map_;

    virtual std::string operator[]( const std::string &str ) const
    {
        auto f = map_.find(str);
        if(f == map_.end())
            return "";

        return f->second;
    }
};


BOOST_AUTO_TEST_CASE( compose_this_man )
{

    {
        auto cp = composer::message("test1record", "#2;test#10;#13;test\\#10;");
        dummy_input ipt("test1record");
        auto rs = cp.format(&ipt);
        BOOST_CHECK_EQUAL("\x02test\x0A\x0Dtest\\#10;", rs);
    }

    dummy_input ipt("testrecord");
    ipt.map_["feld1"] = "103";
    ipt.map_["name"] = "max brause";
    ipt.map_["feld2"] = "407";

    auto cp = composer::message("testrecord", 
            "\\$START $(feld1:%3.3d) $(name:%s)");

    auto result = cp.format(&ipt);
    BOOST_CHECK_EQUAL("$START 103 max brause", result);
}

BOOST_AUTO_TEST_SUITE_END()

