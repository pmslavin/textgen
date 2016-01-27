
class TextGenerator(object):

    map = {}
    nounpairs = [("One","First")]
    maptop = ""

    # LCG constants, Hull-Dobell certified
    m = 2**16
    c = 137
    a = 5

    # Pisano initial state (Galaxy 1!)
    seed0 = 0x5A4A
    seed1 = 0x0248
    seed2 = 0xB753


    def lcg(self):
        xi = self.a*self.x0 + self.c
        xi %= self.m
        self.x0 = xi

        return xi >> 8


    def pisano(self):
        wtemp = self.w0 + self.w1 + self.w2
        wtemp &= (1 << 16) - 1
        self.w0 = self.w1
        self.w1 = self.w2
        self.w2 = wtemp

        return self.w2 >> 8


    def __init__(self, rndGen=pisano):
        self.rndGen = rndGen
        self.noun = ""
        self.adject = ""

        self.w0 = TextGenerator.seed0
        self.w1 = TextGenerator.seed1
        self.w2 = TextGenerator.seed2
        self.x0 = 1


    def gen(self):
        return self.rndGen(self)


    def getNounPair(self):
        return self.nounpairs[self.gen() % len(self.nounpairs)]


    def textGen(self, noun):
        pass




if __name__ == "__main__":

    t = TextGenerator()
    print t.gen()
    print t.getNounPair()

    v = TextGenerator(TextGenerator.lcg)
    print v.gen()
    print v.getNounPair()
