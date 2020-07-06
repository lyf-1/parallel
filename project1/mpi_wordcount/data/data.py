import os


if __name__ == "__main__":
    master_small = open("master_small.txt", 'w')
    master_big = open("master_big.txt", 'w')
    
    files = os.listdir("./")
    for f in files:
        if "small_" in f:
            master_small.write("../data/%s\n" % f)
        elif "big_" in f:
            master_big.write("../data/%s\n" % f)

    master_small.close()
    master_big.close()