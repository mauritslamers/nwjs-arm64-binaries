/*
 * CSS Media Query
 *
 * Copyright (C) 2006 Kimmo Kinnunen <kimmo.t.kinnunen@nokia.com>.
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef MediaQuery_h
#define MediaQuery_h

#include <memory>
#include <utility>

#include "core/CoreExport.h"
#include "platform/heap/Handle.h"
#include "platform/wtf/Vector.h"
#include "platform/wtf/text/StringHash.h"
#include "platform/wtf/text/WTFString.h"

namespace blink {
class MediaQueryExp;

using ExpressionHeapVector = Vector<MediaQueryExp>;

class CORE_EXPORT MediaQuery {
 public:
  enum RestrictorType { kOnly, kNot, kNone };

  static std::unique_ptr<MediaQuery> Create(RestrictorType,
                                            String media_type,
                                            ExpressionHeapVector);
  static std::unique_ptr<MediaQuery> CreateNotAll();

  MediaQuery(RestrictorType, String media_type, ExpressionHeapVector);
  MediaQuery(const MediaQuery&);
  ~MediaQuery();

  RestrictorType Restrictor() const { return restrictor_; }
  const ExpressionHeapVector& Expressions() const { return expressions_; }
  const String& MediaType() const { return media_type_; }
  bool operator==(const MediaQuery& other) const;
  String CssText() const;

  std::unique_ptr<MediaQuery> Copy() const {
    return std::make_unique<MediaQuery>(*this);
  }

 private:
  MediaQuery& operator=(const MediaQuery&) = delete;

  RestrictorType restrictor_;
  String media_type_;
  ExpressionHeapVector expressions_;
  String serialization_cache_;

  String Serialize() const;
};

}  // namespace blink

#endif