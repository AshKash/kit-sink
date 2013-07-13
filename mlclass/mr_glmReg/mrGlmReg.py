'''
Created on Apr 18, 2011

@author: mike-bowles
'''
from mr_glmRegInitialize import MrGlmRegInit1
from mr_glmRegInitialize2 import MrGlmRegInit2
from mr_glmRegIterate import MrGlmRegIter
import simplejson as json
from math import sqrt

def dist(x,y):
    #euclidean distance between two lists    
    sum = 0.0
    for i in range(len(x)):
        temp = x[i] - y[i]
        sum += temp * temp
    return sqrt(sum)


'''
glmnet algo for regression.
sequence of events
1.  make first pass through data to determine means and scale factors
2.  make second pass through data to determine biggest lambda required to suppress all coefficients
3.  decrement lambda (multiply by slightly less than 1).
4.  iteration - do gradient descent passes for each new lambda value

'''

def main():
    #run the first initialization to obtain scale factors
    filePath = '//home//mike-bowles//pyWorkspace//mapReducers//src//mr_glmReg//input.txt'
    mrJob = MrGlmRegInit1(args=[filePath])
    with mrJob.make_runner() as runner:
        runner.run()
    
    
    #run the second initialization to calc lambda starting values
    mrJob = MrGlmRegInit2(args=[filePath])
    with mrJob.make_runner() as runner:
        runner.run()
    
    
    betaPath = "//home//mike-bowles//pyWorkspace//mapReducers//src//mr_glmReg//lambdaBeta.txt"
    fileIn = open(betaPath)
    paramJson = fileIn.read()
    fileIn.close()
    
    
    #Begin iteration to calculate coefficient path
    for i in range(200):
        #run one iteration
        mrJob2 = MrGlmRegIter(args=[filePath])
        with mrJob2.make_runner() as runner:
            runner.run()

if __name__ == '__main__':
    main()