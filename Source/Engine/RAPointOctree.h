#ifndef RAPointOctree_H
#define RAPointOctree_H

#include <vector>

#include "Wm5Vector2.h"
#include "Wm5Vector3.h"
#include "Wm5Vector4.h"
#include "Wm5HMatrix.h"
#include "Wm5APoint.h"
#include "Wm5AVector.h"
#include "Wm5Box3.h"

namespace tyro
{
    class PointOctree
    {
    public:
        PointOctree(const Wm5::Vector3f& origin, const Wm5::Vector3f& halfDimension, const int& maxDepth);
        ~PointOctree();

        // Determine which octant of the tree would contain 'point'
        int getOctantContainingPoint(const Wm5::Vector3f& point) const;
        
        void insert(const Wm5::Vector3f& point, int index);
        
        bool doesIntersectBox2(const Wm5::Vector2f& minBound,
                               const Wm5::Vector2f& maxBound,
                               const Wm5::HMatrix& matrix);
        
        void getAllIntersectedIndicies(const Wm5::Vector2f& minBound,
                                       const Wm5::Vector2f& maxBound,
                                       const Wm5::HMatrix& matrix,
                                       std::vector<std::vector<int>*>& intersected,
                                       std::vector<std::vector<int>*>& nonitersected);
        
        void getDataFromSubtrees(std::vector<std::vector<int>*>& data);
        
        void getSubtrees(std::vector<const PointOctree*>& trees) const;
        
        const std::vector<int>* getDataFromSubtreesContainingPoint(const Wm5::Vector3f& point);


        Wm5::Box3f getBox() const;
        
        void print();
    private:
        Wm5::Vector3f mOrigin;         //The physical center of this node
        Wm5::Vector3f mHalfDimension;  //Half the width/height/depth of this node
//        Wm5::AVector axes[3];
        int mMaxDepth;
        
        //Loose octree
        Wm5::Vector3f mMin;
        Wm5::Vector3f mMax;
        
        PointOctree* mChildren[8]; //! Pointers to child octants
        std::vector<int>* mDataIndicies;
        
        /*
         Children follow a predictable pattern to make accesses simple.
         Here, - means less than 'origin' in that dimension, + means greater than.
         child:	0 1 2 3 4 5 6 7
         x:      - - - - + + + +
         y:      - - + + - - + +
         z:      - + - + - + - +
         */
    };

}

#endif
