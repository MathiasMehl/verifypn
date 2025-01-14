/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   Multiset.h
 * Author: andreas
 *
 * Created on February 20, 2018, 10:37 AM
 */

#ifndef MULTISET_H
#define MULTISET_H

#include <vector>
#include <utility>

#include "Colors.h"


namespace PetriEngine {
    namespace Colored {
        class Multiset {
        private:
            class Iterator {
            private:
                const Multiset* _ms;
                size_t _index;

            public:
                Iterator(const Multiset* ms, size_t index)
                        : _ms(ms), _index(index) {}

                bool operator==(Iterator& other);
                bool operator!=(Iterator& other);
                Iterator& operator++();
                std::pair<const Color*,const uint32_t&> operator++(int);
                std::pair<const Color*, const uint32_t&> operator*();
            };

            typedef std::vector<std::pair<uint32_t,uint32_t>> Internal;

        public:
            Multiset();
            Multiset(const Multiset& orig) = default;
            Multiset(Multiset&&) = default;
            Multiset(std::pair<const Color*,uint32_t> color);
            Multiset(std::vector<std::pair<const Color*,uint32_t>>& colors);
            virtual ~Multiset();

            Multiset& operator=(const Multiset&) = default;
            Multiset& operator=(Multiset&&) = default;

            Multiset operator+ (const Multiset& other) const;
            Multiset operator- (const Multiset& other) const;
            Multiset operator* (uint32_t scalar) const;
            void operator+= (const Multiset& other);
            void operator-= (const Multiset& other);
            void operator*= (uint32_t scalar);
            uint32_t operator[] (const Color* color) const;
            uint32_t& operator[] (const Color* color);

            bool empty() const;
            void clean();

            size_t distinctSize() const {
                return _set.size();
            }

            size_t size() const;

            const Iterator begin() const;
            const Iterator end() const;

            std::string toString() const;

        private:
            Internal _set;
            const ColorType* _type;
        };
    }
}

#endif /* MULTISET_H */

