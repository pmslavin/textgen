# TextGen

## Overview

A utility for generating "random" (but deterministic) textual output
defined by custom grammars.

This is useful in a variety of cases:
 * Testing and Verification: Custom grammars allow text to be generated
   which is relevant to the context being evaluated. This can include:
     - API testing, by generating a full set of client behaviours which
       comprehensively cover an API's facilities.
     - User input fuzzing, by generating a complete set of possible inputs
     - Error handling verification, by deterministically generating known
       error cases.
 * Prompt generation: Grammars can be constructed to serve as prompt generators
   for generative AI, automating the exploration of a large prompt-space in
   a structured way, for example by the presence or absence of keywords/phrases
   in an image or video generation prompt.
 * Dynamic text generation; simple chatbots, NPC dialogue, Elite "Goat Soup" style
   narratives.

The random number generator may be selected from several alternatives with different
characteristics, see [here](#random-number-generators).

Several `operators` are available to modify symbols and generate values within text,
see [here](#operators).


## Requirements

`TextGen` uses the [libjansson](https://github.com/akheron/jansson) library for JSON
parsing.

## Usage

```bash
textgen, version 0.6.0
Usage: textgen [OPTIONS]
    --grammar-file      -g <filename>    The grammar file to load
    --validate-grammar  -v <filename>    The grammar file to validate
    --line-separator    -S <character>   A character used to separate output lines
    --random-seed       -s <seed>        A seed to initialise the random number generator
    --number            -n <number>      The number of texts to generate
    --random-generator  -G <generator>   The random generator to use
    --list-generators   -L               List the available random generators
    --help              -h               Print this help
```

## Grammar Definitions

Grammars are defined as json, in which builtin `operators` may be applied to user-defined
`{symbol}`s.

```json
{
    "__metadata__": {
        "title": "Weather",
        "description": "A grammar to generate weather reports",
        "min_version": "0.3.0"
    },
    "__start__": [
        "{capitalise:prep} {weekday} {period} {adj} {type} will {source} from the {compass}",
        "A {odds} {chance} of {duration} of {adj} {type} exists on {weekday} {period} which will {succeed} {type} later",
        "{weekday} {period} will see {temperature} temperatures followed by {adj} {type}"
    ],
    "prep": [ "on", "during", "throughout"],
    "weekday": [ "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday" ],
    "period": [ "morning", "afternoon", "evening", "night" ],
    "adj": [ "light", "heavy", "intermittent", "persistent", "strong", "gentle", "occasional" ],
    "type": [ "cloud", "rain", "hail", "sleet", "snow", "storms", "showers", "drizzle", "fog", "mist", "frost" ],
    "source": [ "develop", "arrive", "grow", "accumulate" ],
    "compass": [ "north", "east", "south", "west" ],
    "chance": [ "risk", "possibility", "likelihood", "chance", "probability" ],
    "succeed": [ "be followed by", "give way to", "be replaced by", "develop into", "become" ],
    "odds": [ "faint", "strong", "slight", "substantial", "minor", "marked", "moderate" ],
    "duration": [ "periods", "episodes", "spells", "outbreaks" ],
    "temperature": [ "freezing", "cold", "cool", "mild", "temperate", "warm", "hot", "oppressive" ]
}

```
This grammar may then be used to generate texts as follows:
```bash
$ textgen --grammar-file grammars/weather.json --number 4
On Sunday night strong storms will develop from the north.
A moderate risk of periods of persistent hail exists on Wednesday evening which will give way to rain later.
Monday morning will see freezing temperatures followed by occasional fog.
Throughout Sunday morning heavy cloud will accumulate from the west.
```

### Operators

A more complicated grammar illustrates the use of several "operators" to modify symbols
and generate structured values with the text.

```json
{
    "__metadata__": {
        "title": "Users",
        "description": "A grammar to generate json user profiles",
        "min_version": "0.6.0"
    },
    "__start__": [
        "{\"user\": \"{name}\", \"id\": \"{seq:$5d}\", \"role_id\": {range:$1,9}, \"dob\": \"{date:$1920-01-01,2009-12-31}\"}"
    ],
    "name": [ "{capitalise:letter}{capitalise:letter} {capitalise:npref0}{nsuff}",
              "{capitalise:letter}{capitalise:letter} {capitalise:npref0}{npref1}{nsuff}"],
    "letter": [ "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y" ],
    "digit": [ "0", "1", "2", "3", "4", "5", "6", "7", "8", "9" ],
    "npref0": [ "bau", "hen", "tar", "ran", "lar", "whit", "dan", "bur", "mar", "new" ],
    "npref1": [ "un", "san", "ban", "orm", "bin", "ol", "fer", "sim", "in", "en" ],
    "nsuff": [ "ham", "burn", "ton", "son", "dale", "ing", "field", "well", "hold" ]
}
```

This may then be used to generate user-account-style text in the following way:

```bash
$ textgen --grammar-file grammars/users.json -n 8 -S,
{"user": "CP Ranwell", "id": "00000", "role_id": 3, "dob": "1932-10-23"},
{"user": "PJ Ranferfield", "id": "00001", "role_id": 3, "dob": "1972-5-12"},
{"user": "EH Baubandale", "id": "00002", "role_id": 4, "dob": "1975-6-13"},
{"user": "TR Ranwell", "id": "00003", "role_id": 4, "dob": "1972-10-31"},
{"user": "JM Marbinton", "id": "00004", "role_id": 3, "dob": "2003-11-2"},
{"user": "XM Baudale", "id": "00005", "role_id": 2, "dob": "1935-10-25"},
{"user": "FV Newdale", "id": "00006", "role_id": 2, "dob": "1974-4-13"},
{"user": "RJ Larfield", "id": "00007", "role_id": 7, "dob": "1927-1-3"}
```

## Random Number Generators

A variety of random number generators may be selected.
```bash
$ textgen -L
NAME        SEED        DESCRIPTION
rand        %x          The stdlib rand() function, initialised by srand().
constant    %x          Returns a constant value determined by its seed.
cycle       %x:%x       Repeatedly cycles from start_seed to end_seed-1.
pisano16    %x:%x:%x    A 16-bit Pisano period generalised Fibonacci generator
```
