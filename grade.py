import random


def grade():

    return random.randint(50, 100)


res = 0
n = 5
for _ in range(n):
    res += grade()

print(res / n)
