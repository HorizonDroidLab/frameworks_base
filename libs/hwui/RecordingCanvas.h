/*
 * Copyright (C) 2018 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include "CanvasTransform.h"
#include "hwui/Canvas.h"
#include "utils/Macros.h"
#include "utils/TypeLogic.h"

#include "SkCanvas.h"
#include "SkCanvasVirtualEnforcer.h"
#include "SkDrawable.h"
#include "SkNoDrawCanvas.h"
#include "SkPaint.h"
#include "SkPath.h"
#include "SkRect.h"
#include "SkTDArray.h"
#include "SkTemplates.h"

#include <vector>

namespace android {
namespace uirenderer {

enum class DisplayListOpType : uint8_t {
#define X(T) T,
#include "DisplayListOps.in"
#undef X
};

struct DisplayListOp {
    const uint8_t type : 8;
    const uint32_t skip : 24;
};

static_assert(sizeof(DisplayListOp) == 4);

class RecordingCanvas;

class DisplayListData final {
public:
    ~DisplayListData();

    void draw(SkCanvas* canvas) const;

    void reset();
    bool empty() const { return fUsed == 0; }

    void applyColorTransform(ColorTransform transform);

private:
    friend class RecordingCanvas;

    void flush();

    void save();
    void saveLayer(const SkRect*, const SkPaint*, const SkImageFilter*, const SkImage*,
                   const SkMatrix*, SkCanvas::SaveLayerFlags);
    void restore();

    void concat(const SkMatrix&);
    void setMatrix(const SkMatrix&);
    void translate(SkScalar, SkScalar);
    void translateZ(SkScalar);

    void clipPath(const SkPath&, SkClipOp, bool aa);
    void clipRect(const SkRect&, SkClipOp, bool aa);
    void clipRRect(const SkRRect&, SkClipOp, bool aa);
    void clipRegion(const SkRegion&, SkClipOp);

    void drawPaint(const SkPaint&);
    void drawPath(const SkPath&, const SkPaint&);
    void drawRect(const SkRect&, const SkPaint&);
    void drawRegion(const SkRegion&, const SkPaint&);
    void drawOval(const SkRect&, const SkPaint&);
    void drawArc(const SkRect&, SkScalar, SkScalar, bool, const SkPaint&);
    void drawRRect(const SkRRect&, const SkPaint&);
    void drawDRRect(const SkRRect&, const SkRRect&, const SkPaint&);

    void drawAnnotation(const SkRect&, const char*, SkData*);
    void drawDrawable(SkDrawable*, const SkMatrix*);
    void drawPicture(const SkPicture*, const SkMatrix*, const SkPaint*);

    void drawText(const void*, size_t, SkScalar, SkScalar, const SkPaint&);
    void drawPosText(const void*, size_t, const SkPoint[], const SkPaint&);
    void drawPosTextH(const void*, size_t, const SkScalar[], SkScalar, const SkPaint&);
    void drawTextRSXform(const void*, size_t, const SkRSXform[], const SkRect*, const SkPaint&);
    void drawTextBlob(const SkTextBlob*, SkScalar, SkScalar, const SkPaint&);

    void drawImage(sk_sp<const SkImage>, SkScalar, SkScalar, const SkPaint*);
    void drawImageNine(sk_sp<const SkImage>, const SkIRect&, const SkRect&, const SkPaint*);
    void drawImageRect(sk_sp<const SkImage>, const SkRect*, const SkRect&, const SkPaint*,
                       SkCanvas::SrcRectConstraint);
    void drawImageLattice(sk_sp<const SkImage>, const SkCanvas::Lattice&, const SkRect&,
                          const SkPaint*);

    void drawPatch(const SkPoint[12], const SkColor[4], const SkPoint[4], SkBlendMode,
                   const SkPaint&);
    void drawPoints(SkCanvas::PointMode, size_t, const SkPoint[], const SkPaint&);
    void drawVertices(const SkVertices*, const SkVertices::Bone bones[], int boneCount, SkBlendMode,
                      const SkPaint&);
    void drawAtlas(const SkImage*, const SkRSXform[], const SkRect[], const SkColor[], int,
                   SkBlendMode, const SkRect*, const SkPaint*);
    void drawShadowRec(const SkPath&, const SkDrawShadowRec&);

    template <typename T, typename... Args>
    void* push(size_t, Args&&...);

    template <typename Fn, typename... Args>
    void map(const Fn[], Args...) const;

    SkAutoTMalloc<uint8_t> fBytes;
    size_t fUsed = 0;
    size_t fReserved = 0;
};

class RecordingCanvas final : public SkCanvasVirtualEnforcer<SkNoDrawCanvas> {
public:
    RecordingCanvas();
    void reset(DisplayListData*, const SkIRect& bounds);

    sk_sp<SkSurface> onNewSurface(const SkImageInfo&, const SkSurfaceProps&) override;

    void willSave() override;
    SaveLayerStrategy getSaveLayerStrategy(const SaveLayerRec&) override;
    void willRestore() override;

    void onFlush() override;

    void didConcat(const SkMatrix&) override;
    void didSetMatrix(const SkMatrix&) override;
    void didTranslate(SkScalar, SkScalar) override;

    void onClipRect(const SkRect&, SkClipOp, ClipEdgeStyle) override;
    void onClipRRect(const SkRRect&, SkClipOp, ClipEdgeStyle) override;
    void onClipPath(const SkPath&, SkClipOp, ClipEdgeStyle) override;
    void onClipRegion(const SkRegion&, SkClipOp) override;

    void onDrawPaint(const SkPaint&) override;
    void onDrawPath(const SkPath&, const SkPaint&) override;
    void onDrawRect(const SkRect&, const SkPaint&) override;
    void onDrawRegion(const SkRegion&, const SkPaint&) override;
    void onDrawOval(const SkRect&, const SkPaint&) override;
    void onDrawArc(const SkRect&, SkScalar, SkScalar, bool, const SkPaint&) override;
    void onDrawRRect(const SkRRect&, const SkPaint&) override;
    void onDrawDRRect(const SkRRect&, const SkRRect&, const SkPaint&) override;

    void onDrawDrawable(SkDrawable*, const SkMatrix*) override;
    void onDrawPicture(const SkPicture*, const SkMatrix*, const SkPaint*) override;
    void onDrawAnnotation(const SkRect&, const char[], SkData*) override;

    void onDrawText(const void*, size_t, SkScalar x, SkScalar y, const SkPaint&) override;
    void onDrawPosText(const void*, size_t, const SkPoint[], const SkPaint&) override;
    void onDrawPosTextH(const void*, size_t, const SkScalar[], SkScalar, const SkPaint&) override;

    void onDrawTextRSXform(const void*, size_t, const SkRSXform[], const SkRect*,
                           const SkPaint&) override;
    void onDrawTextBlob(const SkTextBlob*, SkScalar, SkScalar, const SkPaint&) override;

    void onDrawBitmap(const SkBitmap&, SkScalar, SkScalar, const SkPaint*) override;
    void onDrawBitmapLattice(const SkBitmap&, const Lattice&, const SkRect&,
                             const SkPaint*) override;
    void onDrawBitmapNine(const SkBitmap&, const SkIRect&, const SkRect&, const SkPaint*) override;
    void onDrawBitmapRect(const SkBitmap&, const SkRect*, const SkRect&, const SkPaint*,
                          SrcRectConstraint) override;

    void onDrawImage(const SkImage*, SkScalar, SkScalar, const SkPaint*) override;
    void onDrawImageLattice(const SkImage*, const Lattice&, const SkRect&, const SkPaint*) override;
    void onDrawImageNine(const SkImage*, const SkIRect&, const SkRect&, const SkPaint*) override;
    void onDrawImageRect(const SkImage*, const SkRect*, const SkRect&, const SkPaint*,
                         SrcRectConstraint) override;

    void onDrawPatch(const SkPoint[12], const SkColor[4], const SkPoint[4], SkBlendMode,
                     const SkPaint&) override;
    void onDrawPoints(PointMode, size_t count, const SkPoint pts[], const SkPaint&) override;
    void onDrawVerticesObject(const SkVertices*, const SkVertices::Bone bones[], int boneCount,
                              SkBlendMode, const SkPaint&) override;
    void onDrawAtlas(const SkImage*, const SkRSXform[], const SkRect[], const SkColor[], int,
                     SkBlendMode, const SkRect*, const SkPaint*) override;
    void onDrawShadowRec(const SkPath&, const SkDrawShadowRec&) override;

private:
    typedef SkCanvasVirtualEnforcer<SkNoDrawCanvas> INHERITED;

    DisplayListData* fDL;
};

};  // namespace uirenderer
};  // namespace android