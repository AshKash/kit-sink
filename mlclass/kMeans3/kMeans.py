'''
Created on Apr 18, 2011

@author: mike-bowles
'''
from mr_kMeansInitialize import MRkMeansInit
from mr_kMeansIterate import MRkMeansIter
import simplejson as json
from math import sqrt

'''
This is a calling program to run several mr jobs
1.  first it calls an mrjob to run through the data set and pick k random points as starting points for iteration
2.  control iterative process by stepping through iterative improvements in centroid calculations until convergence
Improvements - 
a.  more orderly handling of path to intermediate results
b.  really random selection of starting inputs
c.  calculation of SSE
d.  spread reducer calc over multiple reducers. 

'''


def dist(x,y):
    #euclidean distance between two lists    
    sum1 = 0.0
    for i in range(len(x)):
        temp = x[i] - y[i]
        sum1 += temp * temp
    return sqrt(sum1)


def main():
    #first run the initializer to get starting centroids
    filePath = '/Users/ashwin/repos/mlclass/kMeans3/input.txt'
    centPath = "/Users/ashwin/repos/mlclass/kMeans3/"
    
    kMeansStr = '3'
    mrJob = MRkMeansInit([filePath] + ["--k" , kMeansStr  , "--pathName", centPath])
    #mrJob = MRkMeansInit(args=[filePath])
    with mrJob.make_runner() as runner:
        runner.run()
    
    #pull out the centroid values to compare with values after one iteration
    centPath = "/Users/ashwin/repos/mlclass/kMeans3/"
    
    fileIn = open(centPath + "intermediateResults.txt")
    centroidsJson = fileIn.read()
    fileIn.close()
    
    delta = 10
    #Begin iteration on change in centroids
    while delta > 0.01:
        #parse old centroid values
        oldCentroids = json.loads(centroidsJson)
        #run one iteration
        mrJob2 = MRkMeansIter(args=[filePath] + ["--k" , kMeansStr  , "--pathName", centPath])
        with mrJob2.make_runner() as runner:
            runner.run()
            
        #compare new centroids to old ones
        fileIn = open(centPath + "intermediateResults.txt")
        centroidsJson = fileIn.read()
        fileIn.close()
        newCentroids = json.loads(centroidsJson)
        
        kMeans = len(newCentroids)
        
        delta = 0.0
        for i in range(kMeans):
            delta += dist(newCentroids[i],oldCentroids[i])
        
        print delta

if __name__ == '__main__':
    main()
