#include "CCExtractor.h"
#include "highgui.h"
#include "CommonUtilities.h"


CCExtractor::CCExtractor():mapID(0)
{
    reset();
}

void CCExtractor::initialize(int _minSize)
{
    setMinSize(_minSize);
}

void CCExtractor::reset()
{
    initialize(100);
}

void CCExtractor::process(const Mat &binaryImage)
{
    labelImage.create(binaryImage.size(), CV_32SC1);

    clear();
    extractComponents(binaryImage);
    extractConnectedComponents();
    relabelImage();

    //showConnectedComponents();
}

void CCExtractor::clear()
{
    clearComponents();
    clearEquivalenceClassList();
    clearConnectedComponents();
    if (mapID){ delete [] mapID; mapID = 0;}
}

void CCExtractor::clearComponents()
{
    for (vector<Component*>::iterator it = components.begin(), itEnd = components.end(); it != itEnd; it++)
    {
        delete *it;
    }
    components.clear();
    componentCount = 0;
}

void CCExtractor::clearEquivalenceClassList()
{
    for (EquivalenceClassList::iterator it = equivalenceClassList.begin(), itEnd = equivalenceClassList.end(); it != itEnd; it++)
    {
        delete *it;
    }
    equivalenceClassList.clear();
}

void CCExtractor::clearConnectedComponents()
{
    for (vector<ConnectedComponent*>::iterator cc = connectedComponents.begin(), ccEnd = connectedComponents.end(); cc != ccEnd; cc++)
    {
        delete *cc;
    }
    connectedComponents.clear();

    for (vector<ConnectedComponent*>::iterator rcc = rejectedConnectedComponents.begin(), rccEnd = rejectedConnectedComponents.end(); rcc != rccEnd; rcc++)
    {
        delete *rcc;
    }
    rejectedConnectedComponents.clear();
}

void CCExtractor::extractComponents(const Mat & binaryImage)
{
    const unsigned char* bin = binaryImage.ptr<unsigned char>(0);
    int* label = labelImage.ptr<int>(0);
    Component* leftComponent;
    int left = -1;
    Point p;

    // First row
    for ( p.x = 0; p.x < binaryImage.cols; p.x++, bin++, label++)
    {
        if (*bin)   // This pixel is foreground.
        {
            if (left < 0)   // There is no component to the left, so create a new component.
            {
                left = componentCount;
                addComponent(p);
                leftComponent = components.back();
            }
            else    // There is a component to the left, so extend it horizontally.
            {
                leftComponent->addHorizontal(p);
            }
        }
        else left = -1;   // This pixel is background.

        *label = left;
    }

    // All other rows
    for ( p.y = 1; p.y < binaryImage.rows; p.y++ )
    {
        int* top = labelImage.ptr<int>(p.y-1);
        bin = binaryImage.ptr<unsigned char>(p.y);
        label = labelImage.ptr<int>(p.y);
        left = -1;

        for( p.x = 0; p.x < binaryImage.cols; p.x++, bin++, top++, label++ )
        {
            if (*bin)   // This pixel is foreground.
            {
                if (left >= 0)      // The component to the left is being extended horizontally
                {
                    // If there is a different component above, set them equivalent
                    if (*top >= 0 && left != *top) setEquivalence(leftComponent, components[*top]);
                    leftComponent->addHorizontal(p);
                }
                else
                {
                    if (*top >= 0)  // The component above is extended vertically
                    {
                        left = *top;
                        leftComponent = components[left];
                        leftComponent->addVertical(p);
                    }
                    else    // There are no neighbouring components, so create a new one.
                    {
                        left = componentCount;
                        addComponent(p);
                        leftComponent = components.back();
                    }
                }
            }
            else left = -1;    // This pixel is background.

            *label = left;
        }
    }
}

void CCExtractor::extractConnectedComponents()
{
    int id = 0;
    ConnectedComponent* comp;
    mapID = new int[componentCount];

    for (EquivalenceClassList::iterator ecList = equivalenceClassList.begin(), ecListEnd = equivalenceClassList.end(); ecList != ecListEnd; ecList++)
    {
        EquivalenceClass::iterator  ec = (*ecList)->begin(), ecEnd = (*ecList)->end();
        comp = new ConnectedComponent(id, *ecList);

        if (comp->size >= minSize)      // if the connected component is large enough
        {
            connectedComponents.push_back(comp);
            for (; ec != ecEnd; ec++) mapID[(*ec)->id] = id;
            id++;

        }
        else        // the connected component is too small, so reject it
        {
            //delete comp;
            rejectedConnectedComponents.push_back(comp);
            for (; ec != ecEnd; ec++) mapID[(*ec)->id] = -1;
        }
    }
}

void CCExtractor::addComponent(Point & p)
{
    Component* newComponent = new Component(componentCount, p);
    EquivalenceClass* NewEquivalenceClass = new EquivalenceClass();

    newComponent->eqClass = NewEquivalenceClass;
    NewEquivalenceClass->push_back(newComponent);

    components.push_back(newComponent);
    equivalenceClassList.push_back(NewEquivalenceClass);

    componentCount++;
}

void CCExtractor::relabelImage()
{
    // Relabel the label image.  Go from component labels, to connected component labels.

    // Only scan over the CC bounding boxes (there may be some overlap, so a second check must be made)
    vector<ConnectedComponent*>::iterator rcc = rejectedConnectedComponents.begin(), rccEnd = rejectedConnectedComponents.end();
    for (; rcc != rccEnd; rcc++)
    {
        Mat labelROI = labelImage((*rcc)->rect);
        for (MatIterator<int> i(labelROI); !i; ++i)
        {
            if (*i >= 0)
            {
                if (mapID[*i] == -1) *i = -1;
            }
        }
    }

    vector<ConnectedComponent*>::iterator cc = connectedComponents.begin(), ccEnd = connectedComponents.end();
    for (; cc != ccEnd; cc++)
    {
        int id = (*cc)->id;
        Mat labelROI = labelImage((*cc)->rect);
        for (MatIterator<int> i(labelROI); !i; ++i)
        {
            if (*i >= 0)
            {
                if (mapID[*i] == id) *i = id;
            }
        }
    }


}

void CCExtractor::setEquivalence(Component* comp1, Component* comp2)
{
    EquivalenceClass* eq1 = comp1->eqClass;
    EquivalenceClass* eq2 = comp2->eqClass;

    if (eq1 == eq2) return;   // Equivalence has already been established

    EquivalenceClass* l;
    EquivalenceClass* s;

    if (eq1->size() > eq2->size())  {l = eq1; s = eq2;}
    else                            {l = eq2; s = eq1;}

    EquivalenceClass::iterator it = s->begin(), itEnd = s->end();
    for (; it != itEnd; it++) (*it)->eqClass = l;

    l->merge(*s);
    equivalenceClassList.remove(s);
    delete s;
}


void CCExtractor::drawComponents()
{
    int n = components.size();
    Mat R(n,1,CV_8UC3);
    randu(R, Scalar(50,50,50), Scalar(255,255,255));

    Mat image(labelImage.size(), CV_8UC3);

    unsigned char *c;

    // All other rows
    for( int y = 0; y < labelImage.rows; y++ )
    {
        int* p = labelImage.ptr<int>(y);
        unsigned char* i = image.ptr<unsigned char>(y);

        for( int x = 0; x < labelImage.cols; x++ )
        {
            if (*p >= 0)
            {
                c = R.ptr<unsigned char>(*p);
                i[0] = c[0];
                i[1] = c[1];
                i[2] = c[2];
            }
            else
            {
                i[0] = 0;
                i[1] = 0;
                i[2] = 0;
            }
            p++;
            i++; i++; i++;
        }

    }

    imshow("Component Image", image);

}

void CCExtractor::showConnectedComponents()
{
    Mat R(connectedComponents.size(),1,CV_8UC3);
    randu(R, Scalar(50,50,50), Scalar(255,255,255));

    ccImage.create(labelImage.size(), CV_8UC3);

    unsigned char *randColour;

    MatIterator<unsigned char> i(ccImage);
    for (MatIterator<int> l(labelImage); !l; ++l, ++i)
    {
        if (*l < 0)
        {
            i[0] = i[1] = i[2] = 0;
        }
        else
        {
            randColour = R.ptr<unsigned char>(*l);
            i[0] = randColour[0];
            i[1] = randColour[1];
            i[2] = randColour[2];
        }
    }

    // Draw Bounding Boxes and Centroids
    vector<ConnectedComponent*>::iterator cc = connectedComponents.begin(), ccEnd = connectedComponents.end();
    for(; cc != ccEnd; cc++)
    {
        rectangle(ccImage, (*cc)->rect, Scalar(0,0,255));
        line(ccImage, (*cc)->centroid, (*cc)->centroid, Scalar(255,255,255), 3);
    }

    imshow("Connected Component Image", ccImage);

}

void CCExtractor::outputComponents()
{
    log_text("Components:\tsize=%i\n", components.size());

    vector<Component*>::iterator    it = components.begin(),
                                    itEnd = components.end();

    for (; it != itEnd; it++)
    {
        log_text("%i\t", (*it)->id);
    }
    log_text("\n");
}

void CCExtractor::outputEquivalenceClass(EquivalenceClass* eqClass)
{
    log_text("\tEquivalence Class:\tsize=%i\n", eqClass->size());

    EquivalenceClass::iterator it = eqClass->begin(), itEnd = eqClass->end();

    for (; it != itEnd; it++)
    {
        log_text("\t%i\t", (*it)->id);
    }
    log_text("\n");
}

void CCExtractor::outputEquivalenceClasses()
{
    log_text("Equivalence Classes:\tsize=%i\n", equivalenceClassList.size());

    EquivalenceClassList::iterator it = equivalenceClassList.begin(), itEnd = equivalenceClassList.end();

    for (; it != itEnd; it++)
    {
       outputEquivalenceClass(*it);
    }
}
