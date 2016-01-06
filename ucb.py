
import math
import random

strategyDict = {"9pool":999, "10hatch":999, "12hatch":999}

#[winCount, lostCount]
strategyCount = {"9pool":[0, 0], "10hatch":[0, 0], "12hatch":[0, 0]}

totalWin = 0
totalLost = 0

for i in range(1, 50):
    maxStrategy = 0
    maxKey = ""
    for strategy, ucb1 in strategyDict.items():
        if ucb1 >  maxStrategy:
            maxStrategy = ucb1
            maxKey = strategy
    
    randomNum = random.randint(0, 100)
    if maxKey == "9pool":
        if randomNum >= 0:
            result = 1
            strategyCount[maxKey][0] += 1
            totalWin += 1
        else:
            result = 0
            strategyCount[maxKey][1] += 1
            totalLost += 1            
    else:
        result = 0
        strategyCount[maxKey][1] += 1
        totalLost += 1
    
    print "round %d, choose %s, result : %d" %(i, maxKey, result)
    
    print "all strategy UCB1:"
    for strategy, ucb1 in strategyDict.items():
        if strategyCount[strategy][0] + strategyCount[strategy][1] > 0:
            expection = strategyCount[strategy][0] / float(strategyCount[strategy][0] + strategyCount[strategy][1])
            ucb = 0.7 * math.sqrt(math.log(i)/ (strategyCount[strategy][0] + strategyCount[strategy][1]))
            strategyDict[strategy] = expection + ucb
        print "%s : %f" %(strategy, strategyDict[strategy])

print ""
print "total win: %d, total lost: %d" %(totalWin, totalLost)

