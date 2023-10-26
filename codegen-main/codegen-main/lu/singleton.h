// SPDX-License-Identifier: CC0-1.0

#pragma once
#include <type_traits>

namespace lu {
   //
   // Helper class for making singletons. Uses the CRTP. Offers the following features:
   // 
   //  - Disable movement and copying
   // 
   //  - A static `get_or_create` function which constructs an instance, if applicable
   // 
   //  - A static `get` function which acts as a fast getter, assuming the instance is 
   //    already constructed
   //
   template<typename Subclass>
   class singleton {
      protected:
         static Subclass* instance;

         singleton() {
            static_assert(std::is_base_of_v<singleton<Subclass>, Subclass>);
            Subclass::instance = (Subclass*) this;
         }

         // hack to work around the inability to access Subclass::Subclass if it's 
         // protected, without requiring Subclass to friend singleton_ex:
         struct _unprotect : public Subclass {
            _unprotect() : Subclass() {}
         };

      public:
         static Subclass& get_or_create() {
            static _unprotect instance;
            return instance;
         }
         static Subclass& get() {
            return *Subclass::instance;
         }

         singleton(singleton&&) = delete;
         singleton(const singleton&) = delete;

         singleton& operator=(singleton&&) = delete;
         singleton& operator=(const singleton&) = delete;
   };
   template<typename T> T* singleton<T>::instance = nullptr;
}