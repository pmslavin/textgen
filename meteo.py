# LCG constants, Hull-Dobell certified
m = 2**16
c = 137
a = 5

x0 = 1

# Pisano initial state (Galaxy 1!)
seed0 = 0x5A4A
seed1 = 0x0248
seed2 = 0xB753

w0 = seed0
w1 = seed1
w2 = seed2

# todo: temperature
# todo: plagues

map = { "\x80" : ["\x81\xC2. \x81\xC2. \x81\xC2. \x81\xC2"],
        "\x81" : ["\x83\xC0 will experience \x82 \x85 of \x87 \x86", "\xC0 will encounter \x89 \x85 of \x87 \x86", "Although the \xC1 will be \x92 \x8A, a chance of \x82 \x86 exists for \xC0",
                  "WEATHER WARNING: A \x8C exists for \x82 and \x87 \x90 in the \xC1. \x8B", "Despite early \x86, the \xC1 in \xC0 will be \x92 \x8A with \x82 \x86"],
        "\x82" : ["considerable", "lengthy", "sustained", "extended", "brief", "fleeting", "intermittent", "occasional", "frequent"],
        "\x83" : ["In the \xC1, ", "Throughout the \xC1, ", "During the \xC1, ", "As the \xC1 \x84, ", "Towards the end of the \xC1, "],
        "\x84" : ["develops", "progresses", "continues", "passes", "ends"],
        "\x85" : ["periods", "episodes", "intervals"],
        "\x86" : ["rain", "hail", "snow", "showers", "drizzle", "mist", "fog", "frost"],
        "\x87" : ["heavy", "peristent", "light", "intense", "concentrated", "dense"],
        "\x88" : ["be followed by", "give way to", "be replaced by", "become"],
        "\x89" : ["depressing", "miserable", "annoying", "abysmal", "monotonous", "relentless"],
        "\x8A" : ["sunny", "bright", "warm", "mild", "temperate", "pleasant", "calm", "overcast"],
        "\x8B" : ["Residents of \xC0 are advised to \x8E", "Anyone in the vicinity of \xC0 is urged to \x8E", "Occupants of \xC0 may wish to \x8E"],
        "\x8C" : ["risk", "possibility", "potential", "likelihood", "chance"],
        "\x8D" : ["North", "East", "South", "West"], # unused
        "\x8E" : ["carry an umbrella", "avoid \x8F areas", "flee immediately", "move to a more pleasant location", "wear dungarees", "buy a canoe",
                  "wrap up warm", "build a subterranean sanctuary", "perform ritual sacrifices to propitiate the angry gods", "ensure their affairs are in order"],
        "\x8F" : ["coastal", "high", "exposed", "low-lying", "urban", "rural"],
        "\x90" : ["lightning", "storms", "thunder", "monsoons", "tornadoes", "hurricances"],
        "\x91" : ["freezing", "icy", "low", "arctic", "negative", "boiling", "tropical", "high", "sultry"], #unused
        "\x92" : ["largely", "mostly", "essentially", "primarily", "particularly"]
       }

maptop = 0x93
noun = None
times = ["morning", "afternoon", "evening", "night"]
time = times[0]


def textgen(src):
    global time

    out = []
    idx = -1

    while True:
        idx += 1
        try:
            c = src[idx]
        except IndexError:
            break

        oc = ord(c)

        if oc < 128:
            out.append(c)
            continue
        else:
            if oc < maptop:
                rnd = gen()
                mapline = map[chr(oc)]
                inc = rnd % len(mapline)
                ret = textgen(mapline[inc])
                out.append(ret)
                continue

        if oc == 0xC0:
            out.append(noun)
            continue

        if oc == 0xC1:
            out.append(time)
            continue

        if oc == 0xC2:
            time = times[(times.index(time)+1)%len(times)]
            continue


    return "".join(out)



def lcg():
    global x0
    xi = a*x0 + c
    xi %= m
    x0 = xi

    return xi >> 8


def pisano():
    global w0, w1, w2
    wtemp = w0 + w1 + w2
    wtemp &= (1 << 16) - 1
    w0 = w1
    w1 = w2
    w2 = wtemp

    return w2 >> 8


gen = pisano


noun = "Hull"
for _ in xrange(24):
    text = textgen("\x80.")
    print(text)
    print("\n")

