#pragma once

namespace lu::bitstreams {
   class reader;
   class writer;
}

namespace lu {
   template<typename T> concept bitstreamable_struct = requires(T& rx, bitstreams::reader& r, const T& wx, bitstreams::writer & w) {
      { rx.stream(r) };
      { wx.stream(w) };
   };
}
