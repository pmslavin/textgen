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


nounpairs = [ ("Paul Daniels", "Danielian"),
              ("Ocelot", "Ocelotian"),
              ("Badger", "Badgerian"),
              ("Monty", "Montonian"),
              ("The Pope", "Pontifical"),
              ("Kris Akabusi", "Akabusian"),
              ("President Obama", "Presidential")
            ]

adject = ""


def textgen(src):

    map = { "\x80" : ["\x81", "is known for \x84", "never stops \x84 \x87", "is believed to become \x83 through \x85 \x8C"],
            "\x81" : ["is \x82 \x83", "\x82 complains of being \x83", "is \x82 reputed to be \x83", "\x86 \x8C", "\x82 \x86 \x8A \x8B"],
            "\x82" : ["rarely", "frequently", "never", "permanently", "often"],
            "\x83" : ["grumpy", "tired", "paranoid", "excited", "drunk"],
            "\x84" : ["sleeping", "purring", "drinking", "fighting", "moaning"],
            "\x85" : ["an excess of", "habitual", "lack of", "incessant", "occasional"],
            "\x86" : ["enjoys", "dislikes", "abhors", "practises", "loves"],
            "\x87" : ["except to \x88", "unless to \x88", "while \x83", "because he \x86 \x8B"],
            "\x88" : ["\x89 \x8A \x8B", "\x89 \x8B"],
            "\x89" : ["participate in", "indulge in", "practise"],
            "\x8A" : ["violent", "sarcastic", "deadly", "naked", "furious", "delirious", "\xC1"],
            "\x8B" : ["golf", "hopscotch", "cooking", "darts", "pole-vaulting", "terrorism", "flatulence"],
            "\x8C" : ["\x84", "\x8B", "\x8A \x8B"]
           }

    maptop = 0x8D

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
            global adject
            noun, adject = getNounPair()
            out.append(noun)
            continue

        if oc == 0xC1:
            out.append(adject)
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


def getNounPair():
    rnd = gen()
    return nounpairs[rnd%len(nounpairs)]


for _ in xrange(512):
    text = textgen("\xC0 \x80.")
    print(text)

