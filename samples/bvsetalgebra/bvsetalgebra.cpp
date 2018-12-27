/*
Copyright(c) 2002-2017 Anatoliy Kuznetsov(anatoliy_kuznetsov at yahoo.com)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

For more information please visit:  http://bitmagic.io
*/

/** \example bvsetalgebra.cpp
     Example demonstrates variety of algebra of sets operations.
*/

/*! \file bvsetalgebra.cpp
    \brief Example: algebra of sets operations
*/


#include <iostream>
#include <vector>

#include "bm.h"
#include "bmalgo.h"
#include "bmserial.h"
#include "bmaggregator.h"


using namespace std;

// utility function to print a set
static
void print_bvector(const bm::bvector<>& bv)
{
    bm::id_t cnt = 0;
    bm::bvector<>::enumerator en = bv.first();
    for (; en.valid() && cnt < 10; ++en, ++cnt)
        cout << *en << ", ";
    if (cnt == 10)
        cout << " ...";
    cout << "(size = "<< bv.size() << ")" << endl;
}

// utility function to create serialized bit-vector BLOB
static
void make_BLOB(vector<unsigned char>& target_buf, bm::bvector<>& bv)
{
    BM_DECLARE_TEMP_BLOCK(tb)
    bm::serializer<bm::bvector<> > bvs(tb);
    bvs.set_compression_level(4);
    
    bv.optimize(tb, bm::bvector<>::opt_compress); // memory compression

    bm::serializer<bm::bvector<> >::buffer sbuf;
    bvs.serialize(bv, sbuf, 0);
    target_buf.resize(sbuf.size());
    ::memcpy(target_buf.data(), sbuf.buf(), sbuf.size());
}


// -------------------------------------------------------------
// Demo for Set Union (OR) operations
//
static
void DemoOR()
{
    // bit-vector set union operation: bv_A |= bv_B
    {
        bm::bvector<>   bv_A { 1, 2, 3 };
        bm::bvector<>   bv_B { 1, 2, 4 };
        bv_A.bit_or(bv_B);
        
        print_bvector(bv_A); // 1, 2, 3, 4
    }
    // same, but sizes are set, observe size gets extended up
    {
        bm::bvector<>   bv_A { 1, 2, 3 };
        bm::bvector<>   bv_B { 1, 2, 4 };
        bv_A.resize(5);
        bv_B.resize(10);

        bv_A.bit_or(bv_B);
        
        print_bvector(bv_A); // 1, 2, 3, 4 (size = 10)
    }

    // bit-vector set union operation (opcode interpeter mode)
    // maybe useful for building query interpetors
    {
        bm::bvector<>   bv_A { 1, 2, 3 };
        bm::bvector<>   bv_B { 1, 2, 4 };
        bv_A.combine_operation(bv_B, bm::BM_OR);
        
        print_bvector(bv_A); // 1, 2, 3, 4
    }

    // Set union between bit-vector and STL container
    {
        bm::bvector<>      bv_A { 1, 2, 3 };
        vector<unsigned>   vect_B { 1, 2, 4 };
        
        bm::combine_or(bv_A, vect_B.begin(), vect_B.end());
        print_bvector(bv_A); // 1, 2, 3, 4
    }
    
    // Set union between bit-vector and C-array.
    // This tends to be faster then "combine_or()" especially on sorted vectors
    // and in SIMD enabled configurations
    {
        bm::bvector<>      bv_A { 1, 2, 3 };
        vector<unsigned>   vect_B { 1, 2, 4 };
        
        const unsigned* arr = &vect_B[0];
        bv_A.set(arr, unsigned(vect_B.size()), bm::BM_SORTED); // sorted - fastest
        print_bvector(bv_A); // 1, 2, 3, 4
    }

    // Set union between bit-vector and a serialized bit-vector BLOB
    // (created on the fly)
    {
        bm::bvector<>   bv_A { 1, 2, 3 };
        vector<unsigned char> blob;
        {
            bm::bvector<>   bv_B { 1, 2, 4 };
            make_BLOB(blob, bv_B);
        }
        BM_DECLARE_TEMP_BLOCK(tb)
        bm::operation_deserializer<bm::bvector<> >::deserialize(bv_A,
                                                                blob.data(),
                                                                tb,
                                                                bm::set_OR);
        print_bvector(bv_A); // 1, 2, 3, 4
    }
    
    // Union of many sets with bm::aggegator<>
    // target := A OR B OR C
    //
    // This method is best when we have multiple vectors at hands, aggregator
    // is capable of doing it faster, than pair by pair OR
    {
        bm::bvector<>    bv_T; // target vector
        
        bm::bvector<>    bv_A { 1, 2 };
        bm::bvector<>    bv_B { 2, 3 };
        bm::bvector<>    bv_C { 3, 4 };
        
        bm::aggregator<bm::bvector<> > agg;
        agg.set_optimization(); // perform on-the-fly optimization of result
        
        // attach vectors to group 0 for OR operation
        agg.add(&bv_A);
        agg.add(&bv_B);
        agg.add(&bv_C);
        
        agg.combine_or(bv_T);
        
        agg.reset(); // reset the aggregator parameters
        
        print_bvector(bv_T); // 1, 2, 3, 4
    }
}


// -------------------------------------------------------------
// Demo for Set Intersect (AND) operations
//
static
void DemoAND()
{
    // bit-vector set union operation: bv_A |= bv_B
    {
        bm::bvector<>   bv_A { 1, 2, 3 };
        bm::bvector<>   bv_B { 1, 2, 4 };
        bv_A.bit_and(bv_B);
        
        print_bvector(bv_A); // 1, 2
    }
    // same, but sizes are set, observe size gets extended up
    {
        bm::bvector<>   bv_A { 1, 2, 3 };
        bm::bvector<>   bv_B { 1, 2, 4 };
        bv_A.resize(5);
        bv_B.resize(10);

        bv_A.bit_and(bv_B);
        
        print_bvector(bv_A); // 1, 2 (size = 10)
    }

    // bit-vector set union operation (opcode interpeter mode)
    // maybe useful for building query interpetors
    {
        bm::bvector<>   bv_A { 1, 2, 3 };
        bm::bvector<>   bv_B { 1, 2, 4 };
        bv_A.combine_operation(bv_B, bm::BM_AND);
        
        print_bvector(bv_A); // 1, 2
    }

    // Set union between bit-vector and STL container
    {
        bm::bvector<>      bv_A { 1, 2, 3 };
        vector<unsigned>   vect_B { 1, 2, 4 };
        
        bm::combine_and(bv_A, vect_B.begin(), vect_B.end());
        print_bvector(bv_A); // 1, 2
    }
    
    // Set union between bit-vector and a serialized bit-vector BLOB
    //
    {
        bm::bvector<>   bv_A { 1, 2, 3 };
        vector<unsigned char> blob;
        {
            bm::bvector<>   bv_B { 1, 2, 4 };
            make_BLOB(blob, bv_B);
        }
        BM_DECLARE_TEMP_BLOCK(tb)
        bm::operation_deserializer<bm::bvector<> >::deserialize(bv_A,
                                                                blob.data(),
                                                                tb,
                                                                bm::set_AND);
        print_bvector(bv_A); // 1, 2
    }
    
    // Intersection of many sets with bm::aggegator<>
    // target := A AND B AND C
    //
    // This method is best when we have multiple vectors at hands, aggregator
    // is capable of doing it faster, than pair by pair AND
    {
        bm::bvector<>    bv_T; // target vector
        
        bm::bvector<>    bv_A { 1, 2 };
        bm::bvector<>    bv_B { 1, 2, 3 };
        bm::bvector<>    bv_C { 1, 2, 3, 4 };
        
        bm::aggregator<bm::bvector<> > agg;
        agg.set_optimization(); // perform on-the-fly optimization of result
        
        // attach vectors to group 0 for OR operation
        agg.add(&bv_A);
        agg.add(&bv_B);
        agg.add(&bv_C);
        
        agg.combine_and(bv_T);
        
        agg.reset(); // reset the aggregator parameters
        
        print_bvector(bv_T); // 1, 2
    }
}


int main(void)
{
    try
    {
        cout << endl << "Set Union (OR) demo" << endl << endl;;
        DemoOR();
        
        cout << endl << "Set Intersect (AND) demo" << endl << endl;;
        DemoAND();
    }
    catch(std::exception& ex)
    {
        std::cerr << ex.what() << std::endl;
    }


    return 0;
}