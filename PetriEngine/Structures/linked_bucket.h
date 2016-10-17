/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   linked_bucket.h
 * Author: Peter G. Jensen
 *
 * Created on 07 June 2016, 21:51
 */
#include <stdlib.h>
#include <assert.h>
#include <atomic>
#include <vector>
#include <iostream>

#ifndef LINKED_BUCKET_H
#define LINKED_BUCKET_H

template<typename T, size_t C>
class linked_bucket_t {
private:

    struct bucket_t {
        std::atomic<bucket_t*> _nbucket;
        std::atomic<size_t> _offset;
        size_t _count;
        T _data[C];
    } __attribute__ ((aligned (64)));
    
    struct index_t
    {
        bucket_t* _index[C];
        std::atomic<index_t*> _next;
    };

    bucket_t* _begin;
    std::vector<bucket_t* > _tnext;
    index_t* _index;
public:

    linked_bucket_t(size_t threads)
    : _tnext(threads) {
        for (size_t i = 0; i < threads; ++i) {
            _tnext[i] = NULL;
        }
        _begin = new bucket_t;
        _begin->_count = 0;
        _begin->_offset = 0;
        _begin->_nbucket = NULL;
        _tnext[0] = _begin;

        _index = new index_t;
        _index->_next = NULL;

        memset(&_begin->_data, 0, sizeof(T)*C);        
        memset(&_index->_index, 0, sizeof(bucket_t*)*C);
        _index->_index[0] = _begin;
    }

    virtual ~linked_bucket_t() {

        do {
            bucket_t* n = _begin->_nbucket.load();
            delete _begin;
            _begin = n;

        } while (_begin != NULL);

        do {
            index_t* n = _index->_next.load();
            delete _index;
            _index = n;

        } while (_index != NULL);
    }

    inline T& operator[](size_t i) {
        bucket_t* n = indexToBucket(i);
        if(n != NULL)
        {
            return n->_data[i % C];
        }
        
        size_t b = C;
        n = _begin;
        while (b <= i) {
            b += C;
            n = n->_nbucket.load();
            if(n == NULL) std::cerr << "FAILED FETCHING ID: " << i << std::endl;
            assert(n != NULL);
        }

        return n->_data[i % C];
    }

    inline const T& operator[](size_t i) const {

        bucket_t* n = indexToBucket(i);
        if(n != NULL)
        {
            return n->_data[i % C];
        }
        
        n = _begin;        
        size_t b = C;

        while (b <= i) {
            b += C;
            n = n->_nbucket.load();
            assert(n != NULL);
        }

        return n->_data[i % C];
    }

    size_t size() {
        bucket_t* n = _begin;
        size_t cnt = 0;
        while (n != NULL) {
            cnt += n->_count;
            n = n->_nbucket.load();
        }
        return cnt;
    }

    inline size_t next(size_t thread) {
        if (_tnext[thread] == NULL || _tnext[thread]->_count == C) {
            bucket_t* next = new bucket_t;
            next->_count = 0;
            next->_nbucket = NULL;
            next->_offset = 0;
            memset(&next->_data, 0, sizeof(T)*C);
            
            bucket_t* n = _tnext[thread];
            if (n == NULL) {
                n = _begin;
            }

            next->_offset = n->_offset.load() + C; // beginning of next

            bucket_t* tmp = NULL;

            while (!n->_nbucket.compare_exchange_weak(tmp, next)) {
                if (tmp != NULL) {
                    assert(tmp != NULL);
                    n = tmp;
                    next->_offset += C;
                }
            }
            _tnext[thread] = next;

            insertToIndex(next, next->_offset);            
        }

        bucket_t* c = _tnext[thread];

        // return old counter value, then increment
        return c->_offset + (c->_count++);
    }
    
    private:
        
        inline void insertToIndex(bucket_t* bucket, size_t id)
        {
            index_t* tmp = _index;
            while(id >= C*C)
            {
                index_t* old = tmp;
                tmp = old->_next;
                if(tmp == NULL)
                {
                    // extend index if needed
                    index_t* nindex = new index_t;
                    memset(&nindex->_index, 0, sizeof(bucket_t*)*C);
                    nindex->_next = 0;
                    if(!old->_next.compare_exchange_strong(tmp, nindex)) 
                    {
                        delete nindex;
                        tmp = old;
                        continue;
                    }
                    else
                    {
                        tmp = nindex;
                    }
                }
                id -= C*C;
            }
            tmp->_index[id/C] = bucket;
        }
        
        inline bucket_t* indexToBucket(size_t id)
        {
            index_t* tmp = _index;
            while(id >= C*C)
            {
                tmp = tmp->_next;
                if(tmp == NULL) return NULL;
                id -= C*C;
            }
            return tmp->_index[id/C];
        }
} __attribute__ ((aligned (64)));


#endif /* LINKED_BUCKET_H */
