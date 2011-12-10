#pragma once

#include <memory>
#include <list>
#include <vector>
#include <Tools/cv.h>

class Component;

typedef std::shared_ptr<Component> ComponentPtr;
typedef std::list<ComponentPtr> EquivalenceClass;
typedef std::shared_ptr<EquivalenceClass> EquivalenceClassPtr;
typedef std::list<EquivalenceClassPtr> EquivalenceClassList;

class Component
{
public:
    int id;
    int size;
    cv::Point minPt;
    cv::Point maxPt;
    cv::Point pointSum;     // used to calculate the centroid

    EquivalenceClassPtr eqClass;

    Component(int _id, cv::Point & p)
    {
        id = _id;
        size = 1;
        minPt = maxPt = pointSum = p;
    }

    void addHorizontal(cv::Point & p)
    {
        if (p.x > maxPt.x) maxPt.x = p.x;
        size++;
        pointSum.x += p.x;
        pointSum.y += p.y;
    }

    void addVertical(cv::Point & p)
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
    cv::Point centroid;
    cv::Rect rect;

    ConnectedComponent(int _id, EquivalenceClassPtr eqClass)
    {
        id = _id;

        EquivalenceClass::iterator  it = eqClass->begin(),
                                    itEnd = eqClass->end();

        // Initialize values to those of the first component
        ComponentPtr comp = *it;
        size = comp->size;
        cv::Point minPt(comp->minPt);
        cv::Point maxPt(comp->maxPt);
        cv::Point pointSum(comp->pointSum);

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

typedef std::shared_ptr<ConnectedComponent> ConnectedComponentPtr;

class CCExtractor
{
public:

    CCExtractor();

    // Processing
    void process(const cv::Mat & binaryImage);

    // Initialization
    void initialize(int _minSize);
    void reset();

    // Getters
    int getMinSize(){return minSize;}
    
	std::vector<ConnectedComponentPtr> & getConnectedComponents()
	{
		return connectedComponents;
	}

    const cv::Mat& getLabelImage() const { return labelImage; }

    // Setters
    void setMinSize(int _minSize) { minSize = _minSize; }

private:
    // Memory clearing
    void clear();
    void clearComponents();
    void clearEquivalenceClassList();
    void clearConnectedComponents();

    // Processing subroutines
    void extractComponents(const cv::Mat & binaryImage);
    void extractConnectedComponents();
    void relabelImage();
    void setCCMask();
    void addComponent(cv::Point & p);
    void setEquivalence(ComponentPtr comp1, ComponentPtr comp2);

    // Outputing results
    void outputComponents();
    void outputEquivalenceClass(EquivalenceClassPtr eqClass);
    void outputEquivalenceClasses();
    void drawComponents();
    void showConnectedComponents();

    // Output variables
    std::vector<ConnectedComponentPtr> connectedComponents;
    cv::Mat labelImage;
    cv::Mat ccImage;

    // Parameters
    int minSize;

    // Intermediary processing variables
    std::vector<ComponentPtr> components;
    std::vector<ConnectedComponentPtr> rejectedConnectedComponents;
    EquivalenceClassList equivalenceClassList;
    int componentCount;
    int* mapID;
};


