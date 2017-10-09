#include "RAPointOctree.h"
#include "Wm5IntrBox3Box3.h"

namespace RAEngine
{
    PointOctree::PointOctree(const Wm5::Vector3f& origin, const Wm5::Vector3f& halfDimension, const int& maxDepth)
    {
        mOrigin = origin;
        mHalfDimension = halfDimension;
//        axes[0] = AVector::UNIT_X;
//        axes[1] = AVector::UNIT_Y;
//        axes[2] = AVector::UNIT_Z;
        mMaxDepth = maxDepth;
        mDataIndicies = nullptr;

        for (int i=0; i<8; ++i)
        {
            mChildren[i] = nullptr;
        }
    }

    PointOctree::~PointOctree()
    {
        // Recursively destroy octants
        if (mDataIndicies != nullptr)
        {
            delete mDataIndicies;
        }
        
        for (int i = 0; i < 8; ++i)
        {
            delete mChildren[i];
        }
    }

    // Determine which octant of the tree would contain 'point'
    int PointOctree::getOctantContainingPoint(const Wm5::Vector3f& point) const
    {
        int oct = 0;
        if (point.X() >= mOrigin.X()) oct |= 4;
        if (point.Y() >= mOrigin.Y()) oct |= 2;
        if (point.Z() >= mOrigin.Z()) oct |= 1;
        return oct;
    }

    void PointOctree::insert(const Wm5::Vector3f& point, int index)
    {
        // If this node doesn't have a data point yet assigned
        // and it is a leaf, then we're done!
        if (mMaxDepth == 0)
        {
            // Initially, there are no children

            if (mDataIndicies == nullptr) {
                mDataIndicies = new std::vector<int>();
            }
            mDataIndicies->push_back(index);
        }
        else
        {
            if (mChildren[0] == nullptr)
            {
                // Split the current node and create new empty trees for each
                // child octant.
                for (int i = 0; i < 8; ++i)
                {
                    // Compute new bounding box for this child
                    Wm5::APoint newOrigin = mOrigin;
                    newOrigin.X() += mHalfDimension.X() * (i&4 ? .5f : -.5f);
                    newOrigin.Y() += mHalfDimension.Y() * (i&2 ? .5f : -.5f);
                    newOrigin.Z() += mHalfDimension.Z() * (i&1 ? .5f : -.5f);
                    mChildren[i] = new PointOctree(newOrigin, 0.5f * mHalfDimension, mMaxDepth - 1);
                }
            }
            
            mChildren[getOctantContainingPoint(point)]->insert(point, index);
        }
    }

    Wm5::Box3f PointOctree::getBox() const
    {
        Wm5::Vector3f selectionAxis[3] = {Wm5::Vector3f::UNIT_X, Wm5::Vector3f::UNIT_Y, Wm5::Vector3f::UNIT_Z};
        return Wm5::Box3f(mOrigin, selectionAxis, mHalfDimension);
    }

    void PointOctree::print()
    {
        if (mMaxDepth == 0 && mDataIndicies != nullptr)
        {
            printf("###############__New Cell__###############\n");
            for (int j = 0; j < mDataIndicies->size();++j)
            {
                printf("%i\n",(*mDataIndicies)[j]);
            }
            return;
        }
        
        for (int i = 0; i < 8; ++i)
        {
            if (mChildren[i] != nullptr)
            {
                mChildren[i]->print();
            }
        }
    }

    bool PointOctree::doesIntersectBox2(const Wm5::Vector2f& minBound,
                                   const Wm5::Vector2f& maxBound,
                                   const Wm5::HMatrix& matrix)
    {
        using namespace Wm5;
        //convert origin and axis to camera coordinates
        APoint a_mOrigin(mOrigin);
        APoint origin = matrix * a_mOrigin;
        AVector XAxis = matrix * (a_mOrigin + mHalfDimension[0]*AVector::UNIT_X) - origin;
        AVector YAxis = matrix * (a_mOrigin + mHalfDimension[1]*AVector::UNIT_Y) - origin;
        AVector ZAxis = matrix * (a_mOrigin + mHalfDimension[2]*AVector::UNIT_Z) - origin;
        
        Vector3f halfDimension(XAxis.Length(), YAxis.Length(), ZAxis.Length()) ;
        Vector3f axis[3] = {XAxis.GetNormalized(), YAxis.GetNormalized(), ZAxis.GetNormalized()};
        
        Box3f box1 = Box3f(origin, axis, halfDimension);
        
        //Convert 2d box into a 3d by making z-coordinates infinetly big
        Vector2f selectionCenter = (minBound + maxBound)/2;
        Vector3f selectionAxis[3] = {Vector3f::UNIT_X, Vector3f::UNIT_Y, Vector3f::UNIT_Z};
        Vector3f selectionDim(Mathf::FAbs(maxBound.X() - minBound.X())/2,
                              Mathf::FAbs(maxBound.Y() - minBound.Y())/2,
                              FLT_MAX/2);

        Wm5::Box3f box2 = Box3f(Vector3f(selectionCenter.X(), selectionCenter.Y(), 0),
                           selectionAxis,
                           selectionDim);
        
        IntrBox3Box3<float> intrBox3(box1, box2);
        bool doesIntresect = intrBox3.Test();
        return doesIntresect;
    }

    void PointOctree::getAllIntersectedIndicies(const Wm5::Vector2f& minBound,
                                           const Wm5::Vector2f& maxBound,
                                           const Wm5::HMatrix& matrix,
                                           std::vector<std::vector<int>*>& intersected,
                                           std::vector<std::vector<int>*>& nonitersected)
    {
        if (mMaxDepth == 0 && mDataIndicies != nullptr)
        {
            if (this->doesIntersectBox2(minBound, maxBound, matrix))
            {
                intersected.push_back(mDataIndicies);
            }
            else
            {
                nonitersected.push_back(mDataIndicies);
            }
            return;
        }
        
        for (int i = 0; i < 8; ++i)
        {
            if (mChildren[i] != nullptr)
            {
                if (mChildren[i]->doesIntersectBox2(minBound, maxBound, matrix))
                {
                    mChildren[i]->getAllIntersectedIndicies(minBound, maxBound, matrix, intersected, nonitersected);
                }
                else
                {
                    mChildren[i]->getDataFromSubtrees(nonitersected);
                }
            }
        }
    }

    void PointOctree::getDataFromSubtrees(std::vector<std::vector<int>*>& data)
    {
        if (mMaxDepth == 0 && mDataIndicies != nullptr)
        {
            data.push_back(mDataIndicies);
            return;
        }
        
        for (int i = 0; i < 8; ++i)
        {
            if (mChildren[i] != nullptr)
            {
                mChildren[i]->getDataFromSubtrees(data);
            }
        }
    }

    void PointOctree::getSubtrees(std::vector<const PointOctree*>& trees) const
    {
        if (mMaxDepth == 0 && mDataIndicies != nullptr)
        {
            trees.push_back(this);
            return;
        }
        
        for (int i = 0; i < 8; ++i)
        {
            if (mChildren[i] != nullptr)
            {
                mChildren[i]->getSubtrees(trees);
            }
        }
        
        trees.push_back(this);
    }


    const std::vector<int>* PointOctree::getDataFromSubtreesContainingPoint(const Wm5::Vector3f& point)
    {
        // If this node doesn't have a data point yet assigned
        // and it is a leaf, then we're done!
        if (mMaxDepth == 0)
        {
            if (mDataIndicies != nullptr && mDataIndicies->size() > 0)
            {
                return mDataIndicies;
            }
            else
            {
                return nullptr;
            }
        }
        else
        {
            int octant = getOctantContainingPoint(point);
            if (mChildren[octant] != nullptr)
            {
                return mChildren[octant]->getDataFromSubtreesContainingPoint(point);
            }
            else
            {
                return nullptr;
            }
            
        }

    }

}




