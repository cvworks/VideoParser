#ifndef CCEXTRACTOR_H
#define CCEXTRACTOR_H

#include <list>
#include <vector>
#include "core/core.hpp"
#include "Mask.h"

using namespace std;
using namespace cv;

struct Component;

typedef list<Component*> EquivalenceClass;
typedef list<EquivalenceClass*> EquivalenceClassList;

struct Component
{
    int id;
    int size;
    Point minPt;
    Point maxPt;
    Point pointSum;     // used to calculate the centroid

    EquivalenceClass* eqClass;

    Component(int _id, Point & p)
    {
        id = _id;
        size = 1;
        minPt = maxPt = pointSum = p;
    }

    void addHorizontal(Point & p)
    {
        if (p.x > maxPt.x) maxPt.x = p.x;
        size++;
        pointSum.x += p.x;
        pointSum.y += p.y;
    }

    void addVertical(Point & p)
    {
        maxPt.y = p.y;
        size++;
        pointSum.x += p.x;
        pointSum.y += p.y;
    }
};


struct ConnectedComponent
{
    int id;
    int size;
    Point centroid;
    Rect rect;

    ConnectedComponent(int _id, EquivalenceClass* eqClass)
    {
        id = _id;

        EquivalenceClass::iterator  it = eqClass->begin(),
                                    itEnd = eqClass->end();

        // Initialize values to those of the first component
        Component* comp = *it;
        size = comp->size;
        Point minPt(comp->minPt);
        Point maxPt(comp->maxPt);
        Point pointSum(comp->pointSum);

        it++;
        // For every other component...
        for (; it != itEnd; it++)
        {
            //(*it)->id = id;
            comp = *it;
            size += comp->size;
            if (comp->minPt.x < minPt.x) minPt.x = comp->minPt.x;
            if (comp->minPt.y < minPt.y) minPt.y = comp->minPt.y;
            if (comp->maxPt.x > maxPt.x) maxPt.x = comp->maxPt.x;
            if (comp->maxPt.y > maxPt.y) maxPt.y = comp->maxPt.y;
            pointSum.x += comp->pointSum.x;
            pointSum.y += comp->pointSum.y;
        }

        // Set the bounding box
        rect.x = minPt.x;
        rect.y = minPt.y;
        rect.width = maxPt.x - minPt.x + 1;
        rect.height = maxPt.y - minPt.y + 1;

        // Set the centroid
        centroid.x = pointSum.x / size;
        centroid.y = pointSum.y / size;
    }
};

class CCExtractor
{
public:

    CCExtractor();

    // Processing
    void process(const Mat & binaryImage);

    // Initialization
    void initialize(int _minSize);
    void reset();

    // Getters
    int getMinSize(){return minSize;}
    vector<ConnectedComponent*> & getConnectedComponents(){return connectedComponents;}
    Mat getLabelImage(){return labelImage;}

    // Setters
    void setMinSize(int _minSize){minSize = _minSize;}

private:
    // Memory clearing
    void clear();
    void clearComponents();
    void clearEquivalenceClassList();
    void clearConnectedComponents();

    // Processing subroutines
    void extractComponents(const Mat & binaryImage);
    void extractConnectedComponents();
    void relabelImage();
    void setCCMask();
    void addComponent(Point & p);
    void setEquivalence(Component* comp1, Component* comp2);

    // Outputing results
    void outputComponents();
    void outputEquivalenceClass(EquivalenceClass* eqClass);
    void outputEquivalenceClasses();
    void drawComponents();
    void showConnectedComponents();

    // Output variables
    vector<ConnectedComponent*> connectedComponents;
    Mat labelImage;
    Mat ccImage;

    // Parameters
    int minSize;

    // Intermediary processing variables
    vector<Component*> components;
    vector<ConnectedComponent*> rejectedConnectedComponents;
    EquivalenceClassList equivalenceClassList;
    int componentCount;
    int* mapID;
};

#endif // CCEXTRACTOR_H
