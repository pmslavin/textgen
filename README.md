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


## Requirements

`TextGen` uses the [libjansson](https://github.com/akheron/jansson) library for JSON
parsing.

## Usage

```bash
textgen, version 0.5.0
Usage: textgen [OPTIONS]
    --grammar-file      -g <filename>    The grammar file to load
    --validate-grammar  -v <filename>    The grammar file to validate
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

## Random Number Generators

A variety of random number generators may be selected.
```bash
$ ./build/textgen -L
NAME        SEED        DESCRIPTION
rand        %x          The stdlib rand() function, initialised by srand().
constant    %x          Returns a constant value determined by its seed.
cycle       %x:%x       Repeatedly cycles from start_seed to end_seed-1.
pisano16    %x:%x:%x    A 16-bit Pisano period generalised Fibonacci generator
```
