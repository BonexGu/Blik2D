﻿#pragma once
#include <platform/blik_platform.hpp>
#include <element/blik_clip.hpp>
#include <element/blik_rect.hpp>
#include <element/blik_point.hpp>
#include <element/blik_size.hpp>
#include <element/blik_vector.hpp>
#include <element/blik_color.hpp>
#include <element/blik_image.hpp>
#include <element/blik_font.hpp>
#include <functional>

// 옵션스택관련
#define ZAY_LTRB(PANEL, L, T, R, B) \
    if(auto _ = (PANEL)._push_clip(L, T, R, B, false))
#define ZAY_LTRB_UI(PANEL, L, T, R, B, ...) \
    if(auto _ = (PANEL)._push_clip_ui(L, T, R, B, false, __VA_ARGS__))
#define ZAY_LTRB_SCISSOR(PANEL, L, T, R, B) \
    if(auto _ = (PANEL)._push_clip(L, T, R, B, true))
#define ZAY_LTRB_UI_SCISSOR(PANEL, L, T, R, B, ...) \
    if(auto _ = (PANEL)._push_clip_ui(L, T, R, B, true, __VA_ARGS__))

#define ZAY_XYWH(PANEL, X, Y, W, H) \
    if(auto _ = (PANEL)._push_clip(X, Y, (X) + (W), (Y) + (H), false))
#define ZAY_XYWH_UI(PANEL, X, Y, W, H, ...) \
    if(auto _ = (PANEL)._push_clip_ui(X, Y, (X) + (W), (Y) + (H), false, __VA_ARGS__))
#define ZAY_XYWH_SCISSOR(PANEL, X, Y, W, H) \
    if(auto _ = (PANEL)._push_clip(X, Y, (X) + (W), (Y) + (H), true))
#define ZAY_XYWH_UI_SCISSOR(PANEL, X, Y, W, H, ...) \
    if(auto _ = (PANEL)._push_clip_ui(X, Y, (X) + (W), (Y) + (H), true, __VA_ARGS__))

#define ZAY_XYRR(PANEL, X, Y, RW, RH) \
    if(auto _ = (PANEL)._push_clip((X) - (RW), (Y) - (RH), (X) + (RW), (Y) + (RH), false))
#define ZAY_XYRR_UI(PANEL, X, Y, RW, RH, ...) \
    if(auto _ = (PANEL)._push_clip_ui((X) - (RW), (Y) - (RH), (X) + (RW), (Y) + (RH), false, __VA_ARGS__))
#define ZAY_XYRR_SCISSOR(PANEL, X, Y, RW, RH) \
    if(auto _ = (PANEL)._push_clip((X) - (RW), (Y) - (RH), (X) + (RW), (Y) + (RH), true))
#define ZAY_XYRR_UI_SCISSOR(PANEL, X, Y, RW, RH, ...) \
    if(auto _ = (PANEL)._push_clip_ui((X) - (RW), (Y) - (RH), (X) + (RW), (Y) + (RH), true, __VA_ARGS__))

#define ZAY_RECT(PANEL, R) \
    if(auto _ = (PANEL)._push_clip_by_rect(R, false))
#define ZAY_RECT_UI(PANEL, R, ...) \
    if(auto _ = (PANEL)._push_clip_ui_by_rect(R, false, __VA_ARGS__))
#define ZAY_RECT_SCISSOR(PANEL, R) \
    if(auto _ = (PANEL)._push_clip_by_rect(R, true))
#define ZAY_RECT_UI_SCISSOR(PANEL, R, ...) \
    if(auto _ = (PANEL)._push_clip_ui_by_rect(R, true, __VA_ARGS__))

#define ZAY_INNER(PANEL, V) \
    if(auto _ = (PANEL)._push_clip(V, V, (PANEL).w() - (V), (PANEL).h() - (V), false))
#define ZAY_INNER_UI(PANEL, V, ...) \
    if(auto _ = (PANEL)._push_clip_ui(V, V, (PANEL).w() - (V), (PANEL).h() - (V), false, __VA_ARGS__))
#define ZAY_INNER_SCISSOR(PANEL, V) \
    if(auto _ = (PANEL)._push_clip(V, V, (PANEL).w() - (V), (PANEL).h() - (V), true))
#define ZAY_INNER_UI_SCISSOR(PANEL, V, ...) \
    if(auto _ = (PANEL)._push_clip_ui(V, V, (PANEL).w() - (V), (PANEL).h() - (V), true, __VA_ARGS__))

#define ZAY_CHILD(PANEL) \
    if(auto _ = (PANEL)._push_clip_by_child(0, 0, 0xFFFF, 0xFFFF, false))
#define ZAY_CHILD_UI(PANEL, ...) \
    if(auto _ = (PANEL)._push_clip_ui_by_child(0, 0, 0xFFFF, 0xFFFF, false, __VA_ARGS__))
#define ZAY_CHILD_SCISSOR(PANEL) \
    if(auto _ = (PANEL)._push_clip_by_child(0, 0, 0xFFFF, 0xFFFF, true))
#define ZAY_CHILD_UI_SCISSOR(PANEL, ...) \
    if(auto _ = (PANEL)._push_clip_ui_by_child(0, 0, 0xFFFF, 0xFFFF, true, __VA_ARGS__))

#define ZAY_CHILD_AT(PANEL, IX, IY) \
    if(auto _ = (PANEL)._push_clip_by_child(IX, IY, 1, 1, false))
#define ZAY_CHILD_AT_UI(PANEL, IX, IY, ...) \
    if(auto _ = (PANEL)._push_clip_ui_by_child(IX, IY, 1, 1, false, __VA_ARGS__))
#define ZAY_CHILD_AT_SCISSOR(PANEL, IX, IY) \
    if(auto _ = (PANEL)._push_clip_by_child(IX, IY, 1, 1, true))
#define ZAY_CHILD_AT_UI_SCISSOR(PANEL, IX, IY, ...) \
    if(auto _ = (PANEL)._push_clip_ui_by_child(IX, IY, 1, 1, true, __VA_ARGS__))

#define ZAY_CHILD_SECTION(PANEL, IX, IY, XC, YC) \
    if(auto _ = (PANEL)._push_clip_by_child(IX, IY, XC, YC, false))
#define ZAY_CHILD_SECTION_UI(PANEL, IX, IY, XC, YC, ...) \
    if(auto _ = (PANEL)._push_clip_ui_by_child(IX, IY, XC, YC, false, __VA_ARGS__))
#define ZAY_CHILD_SECTION_SCISSOR(PANEL, IX, IY, XC, YC) \
    if(auto _ = (PANEL)._push_clip_by_child(IX, IY, XC, YC, true))
#define ZAY_CHILD_SECTION_UI_SCISSOR(PANEL, IX, IY, XC, YC, ...) \
    if(auto _ = (PANEL)._push_clip_ui_by_child(IX, IY, XC, YC, true, __VA_ARGS__))

#define ZAY_RGB(PANEL, R, G, B) \
    if(auto _ = (PANEL)._push_color(R, G, B, 0xFF))
#define ZAY_RGB_IF(PANEL, R, G, B, QUERY) \
    if(auto _ = (QUERY)? (PANEL)._push_color(R, G, B, 0xFF) : (PANEL)._push_pass())
#define ZAY_RGBA(PANEL, R, G, B, A) \
    if(auto _ = (PANEL)._push_color(R, G, B, A))
#define ZAY_RGBA_IF(PANEL, R, G, B, A, QUERY) \
    if(auto _ = (QUERY)? (PANEL)._push_color(R, G, B, A) : (PANEL)._push_pass())
#define ZAY_COLOR(PANEL, COLOR) \
    if(auto _ = (PANEL)._push_color(COLOR))
#define ZAY_COLOR_IF(PANEL, COLOR, QUERY) \
    if(auto _ = (QUERY)? (PANEL)._push_color(COLOR) : (PANEL)._push_pass())
#define ZAY_COLOR_CLEAR(PANEL) \
    if(auto _ = (PANEL)._push_color_clear())

#define ZAY_FONT(PANEL, ...) \
    if(auto _ = (PANEL)._push_font(__VA_ARGS__))
#define ZAY_ZOOM(PANEL, ZOOM) \
    if(auto _ = (PANEL)._push_zoom(ZOOM))

// 서브패널관련
#define ZAY_MAKE_SUB(PANEL, SURFACE) \
    for(ZayPanel PANEL(SURFACE, Platform::Graphics::GetSurfaceWidth(SURFACE), Platform::Graphics::GetSurfaceHeight(SURFACE)); (PANEL)._is_dirty(); (PANEL)._clear_me())
#define ZAY_MAKE_SUB_UI(PANEL, SURFACE, NAME) \
    for(ZayPanel PANEL(SURFACE, Platform::Graphics::GetSurfaceWidth(SURFACE), Platform::Graphics::GetSurfaceHeight(SURFACE), NAME); (PANEL)._is_dirty(); (PANEL)._clear_me())
#define ZAY_MAKE_SUB_WH(PANEL, SURFACE, W, H) \
    for(ZayPanel PANEL(SURFACE, W, H); (PANEL)._is_dirty(); (PANEL)._clear_me())
#define ZAY_MAKE_SUB_UI_WH(PANEL, SURFACE, NAME, W, H) \
    for(ZayPanel PANEL(SURFACE, W, H, NAME); (PANEL)._is_dirty(); (PANEL)._clear_me())

// 뷰등록관련
#define ZAY_VIEW_API static void
#define ZAY_DECLARE_VIEW(NAME) ZAY_DECLARE_VIEW_CLASS(NAME, ZayObject)
#define ZAY_DECLARE_VIEW_CLASS(NAME, CLASS) \
    ZAY_VIEW_API OnCommand(CommandType, chars, id_share, id_cloned_share*); \
    ZAY_VIEW_API OnNotify(chars, chars, id_share, id_cloned_share*); \
    ZAY_VIEW_API OnGesture(GestureType, sint32, sint32); \
    ZAY_VIEW_API OnRender(ZayPanel&); \
    static ZayInstance<CLASS> m; \
    static void _Bind(ZayObject* ptr) {m._bind((CLASS*) ptr);} \
    static ZayObject* _Alloc() {return (ZayObject*) Buffer::Alloc<CLASS>(BLIK_DBG 1);} \
    static void _Free(ZayObject* ptr) {Buffer::Free((buffer) ptr);} \
    static autorun _ = ZayView::_makefunc(false, "" NAME, OnCommand, OnNotify, \
        OnGesture, OnRender, _Bind, _Alloc, _Free);
#define ZAY_DECLARE_VIEW_NATIVE(NAME, CLASS) \
    ZAY_VIEW_API OnCommand(CommandType, chars, id_share, id_cloned_share*); \
    ZAY_VIEW_API OnNotify(chars, chars, id_share, id_cloned_share*); \
    ZAY_VIEW_API OnGesture(GestureType, sint32, sint32) {BLIK_ASSERT("This function should not be called directly.", false);} \
    ZAY_VIEW_API OnRender(ZayPanel&) {BLIK_ASSERT("This function should not be called directly.", false);} \
    static ZayInstance<CLASS> m; \
    static void _Bind(ZayObject* ptr) {m._bind((CLASS*) ptr);} \
    static ZayObject* _Alloc() {return (ZayObject*) new CLASS;} \
    static void _Free(ZayObject* ptr) {delete (CLASS*) ptr;} \
    static autorun _ = ZayView::_makefunc(true, "" NAME, OnCommand, OnNotify, \
        OnGesture, OnRender, _Bind, _Alloc, _Free);

// UI제스처 콜백함수
#define ZAY_GESTURE_T(TYPE, ...)                       [__VA_ARGS__](ZayObject*, chars, GestureType TYPE, sint32, sint32)->void
#define ZAY_GESTURE_TXY(TYPE, X, Y, ...)               [__VA_ARGS__](ZayObject*, chars, GestureType TYPE, sint32 X, sint32 Y)->void
#define ZAY_GESTURE_NT(NAME, TYPE, ...)                [__VA_ARGS__](ZayObject*, chars NAME, GestureType TYPE, sint32, sint32)->void
#define ZAY_GESTURE_NTXY(NAME, TYPE, X, Y, ...)        [__VA_ARGS__](ZayObject*, chars NAME, GestureType TYPE, sint32 X, sint32 Y)->void
#define ZAY_GESTURE_VNT(VIEW, NAME, TYPE, ...)         [__VA_ARGS__](ZayObject* VIEW, chars NAME, GestureType TYPE, sint32, sint32)->void
#define ZAY_GESTURE_VNTXY(VIEW, NAME, TYPE, X, Y, ...) [__VA_ARGS__](ZayObject* VIEW, chars NAME, GestureType TYPE, sint32 X, sint32 Y)->void

namespace BLIK
{
    enum CommandType {CT_Create, CT_CanQuit, CT_Destroy, CT_Size, CT_Tick, CT_Signal};
    enum GestureType {
        // 일반
        GT_Null, GT_Moving, GT_MovingLosed, GT_Pressed,
        GT_InDragging, GT_OutDragging, GT_InReleased, GT_OutReleased,
        GT_Dropping, GT_DroppingLosed, GT_Dropped,
        // 마우스휠
        GT_WheelUp, GT_WheelDown, GT_WheelPress, GT_WheelDragging, GT_WheelRelease,
        // 마우스확장
        GT_ExtendPress, GT_ExtendDragging, GT_ExtendRelease,
        // 마우스휠피크(자식패널에게 이벤트가 전달될 경우, 최상단인 OnPanel에도 전달)
        GT_WheelUpPeeked, GT_WheelDownPeeked, GT_WheelPressPeeked, GT_WheelDraggingPeeked, GT_WheelReleasePeeked,
        // 마우스확장피크(자식패널에게 이벤트가 전달될 경우, 최상단인 OnPanel에도 전달)
        GT_ExtendPressPeeked, GT_ExtendDraggingPeeked, GT_ExtendReleasePeeked,
        // 특수
        GT_ToolTip, GT_LongPressed};
    enum PanelState {PS_Null = 0x00, PS_Hovered = 0x01, PS_Focused = 0x02,
        PS_Pressed = 0x04, PS_Dragging = 0x08, PS_Dropping = 0x10};
    static PanelState operator|(PanelState a, PanelState b) {return (PanelState) (int(a) | int(b));}
    enum SynchronizeModel {SM_Null, SM_UpdateAll, // 전체 동기화
        SM_Rename, SM_Move, SM_Modify, // 자신의 에디트
        SM_AddChild, SM_InsertChild, SM_RemoveChild, // 자식의 개체관리
        SM_RenameChild, SM_MoveChild, SM_ModifyChild}; // 자식의 에디트

    //! \brief 뷰의 자료관리
    class ZayObject
    {
        friend class ZayView;
        friend class ZayController;

    public:
        typedef void (*CommandCB)(CommandType, chars, id_share, id_cloned_share*);
        typedef void (*NotifyCB)(chars, chars, id_share, id_cloned_share*);
        typedef void (*BindCB)(ZayObject*);
        typedef ZayObject* (*AllocCB)();
        typedef void (*FreeCB)(ZayObject*);

    public:
        typedef const void* (*FinderCB)(void*, chars);
        typedef void (*UpdaterCB)(void*, sint32);

    public:
        ZayObject();
        virtual ~ZayObject();

    public:
        void bind(void* resource);
        virtual void initialize();
        virtual void synchronize(SynchronizeModel model, sint32 param) const;

    public:
        void invalidate(sint32 count = 1) const;
        void invalidate(chars uigroup) const;
    protected:
        static void invalidator(payload data, chars uigroup);

    public:
        ZayObject* next(chars viewclass);
        bool next(View* viewmanager);
        void exit();
        bool valid(chars uiname) const;
        const rect128& rect(chars uiname) const;
        const float zoom(chars uiname) const;
        const point64& oldxy(chars uiname) const;
        void resizeForced(sint32 w = -1, sint32 h = -1);
        bool getResizingValue(sint32& w, sint32& h);

    public:
        inline h_view view() const
        {return m_view;}
        inline sint32 frame() const
        {return m_frame_counter;}
        inline FrameUpdater* updater()
        {return &m_frame_updater;}

    private:
        h_view m_view;
        void* m_resource;
        sint32 m_frame_counter;
        FrameUpdater m_frame_updater;
        FinderCB m_finder;
        void* m_finder_data;
        UpdaterCB m_updater;
        void* m_updater_data;
        sint32 m_resizing_width;
        sint32 m_resizing_height;
    };

    //! \brief 뷰의 패널관리
    class ZayPanel
    {
    public:
        typedef void (*GestureCB)(GestureType, sint32, sint32);
        typedef std::function<void(ZayObject*, chars, GestureType, sint32, sint32)> SubGestureCB;
        typedef void (*RenderCB)(ZayPanel&);

    public:
        ZayPanel(float width, float height, const buffer touch);
        ZayPanel(id_surface surface, float width, float height, chars uigroup = nullptr);
        ~ZayPanel();

    public:
        void erase() const;
        void fill() const;
        void rect(float thick) const;
        void line(const Point& begin, const Point& end, float thick) const;
        void circle() const;
        void bezier(const Vector& begin, const Vector& end, float thick) const;
        void polygon(Points p) const;
        void polyline(Points p, float thick) const;
        void polybezier(Points p, float thick, bool showfirst, bool showlast) const;
        haschild icon(const Image& image, UIAlign align, bool visible = true);
        haschild icon(float x, float y, const Image& image, UIAlign align, bool visible = true); // 중점식
        haschild iconNative(id_image_read image, UIAlign align);
        haschild iconNative(float x, float y, id_image_read image, UIAlign align); // 중점식
        haschild stretch(const Image& image, bool rebuild, bool visible = true);
        haschild stretchNative(id_image_read image) const;
        haschild ninepatch(const Image& image, bool visible = true);
        void pattern(const Image& image, UIAlign align, bool reversed_xorder = false, bool reversed_yorder = false) const;
        bool text(chars string, UIFontAlign align, UIFontElide elide = UIFE_None) const;
        void text(float x, float y, chars string, UIFontAlign align) const; // 중점식
        void sub(chars uigroup, id_surface surface) const;
        PanelState state(chars uiname = nullptr) const;
        Point toview(float x, float y) const;
        void test(UITestOrder order);

    public:
        inline const float w() const
        {return m_clipped_width;}
        inline const float h() const
        {return m_clipped_height;}
        inline const float zoom() const
        {return m_stack_zoom[-1];}

    public:
        enum StackType {ST_Null, ST_Pass, ST_Clip, ST_Color, ST_Font, ST_Zoom};
        class StackBinder
        {
            BLIK_DECLARE_NONCOPYABLE_INITIALIZED_CLASS(StackBinder, mPanel(rhs.mPanel))
            friend class ZayPanel;
        private:
            StackBinder(ZayPanel* panel, StackType type = ST_Null) : mPanel(panel) {mType = type;}
        public:
            ~StackBinder()
            {
                switch(mType)
                {
                case ST_Clip: mPanel->_pop_clip(); break;
                case ST_Color: mPanel->_pop_color(); break;
                case ST_Font: mPanel->_pop_font(); break;
                case ST_Zoom: mPanel->_pop_zoom(); break;
                }
            }
        public:
            StackBinder(StackBinder&& rhs) : mPanel(rhs.mPanel) {mType = rhs.mType; rhs.mType = ST_Null;}
            StackBinder& operator=(StackBinder&& rhs) {mType = rhs.mType; rhs.mType = ST_Null; return *this;}
            operator bool() const {return (mType != ST_Null);}
        private:
            ZayPanel* const mPanel;
            StackType mType;
        };

    public:
        StackBinder _push_clip(float l, float t, float r, float b, bool doScissor);
        StackBinder _push_clip_ui(float l, float t, float r, float b, bool doScissor, chars uiname, SubGestureCB cb = nullptr, bool hoverpass = true);
        StackBinder _push_clip_by_rect(const Rect& r, bool doScissor);
        StackBinder _push_clip_ui_by_rect(const Rect& r, bool doScissor, chars uiname, SubGestureCB cb = nullptr, bool hoverpass = true);
        StackBinder _push_clip_by_child(sint32 ix, sint32 iy, sint32 xcount, sint32 ycount, bool doScissor);
        StackBinder _push_clip_ui_by_child(sint32 ix, sint32 iy, sint32 xcount, sint32 ycount, bool doScissor, chars uiname, SubGestureCB cb = nullptr, bool hoverpass = true);
        StackBinder _push_color(sint32 r, sint32 g, sint32 b, sint32 a);
        StackBinder _push_color(const Color& color);
        StackBinder _push_color_clear();
        StackBinder _push_font(float size, chars name = nullptr);
        StackBinder _push_zoom(float zoom);
        StackBinder _push_pass();
    private:
        void _pop_clip();
        void _pop_color();
        void _pop_font();
        void _pop_zoom();

    private:
        bool _push_scissor(float l, float t, float r, float b);
        void _pop_scissor();

    public:
        inline bool _is_dirty() const
        {return m_dirty;}
        inline void _clear_me()
        {m_dirty = false;}

    protected:
        bool m_dirty;
        const float m_width;
        const float m_height;
        id_surface m_ref_surface;
        void* m_ref_touch;
        void* m_ref_touch_collector;
        Clips m_stack_clip;
        Rects m_stack_scissor;
        Colors m_stack_color;
        Fonts m_stack_font;
        floats m_stack_zoom;

    protected:
        float m_clipped_width;
        float m_clipped_height;
        const Image* m_child_image;
        Rect m_child_guide;
        bool m_test_scissor;
    };

    //! \brief 뷰의 인스턴스관리
    template<typename TYPE>
    class ZayInstance
    {
    public:
        ZayInstance()
        {
            m_ref_data_last = nullptr;
        }
        ~ZayInstance()
        {
        }

    public:
        inline TYPE* operator->()
        {
            BLIK_ASSERT("현재 위치에서는 m이 사용될 수 없습니다", m_ref_data_last);
            return m_ref_data_last;
        }
        inline TYPE& operator()()
        {
            BLIK_ASSERT("현재 위치에서는 m이 사용될 수 없습니다", m_ref_data_last);
            return *m_ref_data_last;
        }
        inline void _bind(TYPE* ptr)
        {
            if(ptr)
            {
                m_ref_datas.AtAdding() = ptr;
                m_ref_data_last = ptr;
            }
            else
            {
                m_ref_datas.SubtractionOne();
                if(0 < m_ref_datas.Count())
                    m_ref_data_last = m_ref_datas[-1];
                else m_ref_data_last = nullptr;
            }
        }

    private:
        Array<TYPE*, datatype_pod_canmemcpy> m_ref_datas;
        TYPE* m_ref_data_last;
    };

    //! \brief 제이뷰
    class ZayView : public View
    {
        friend class ZayObject;
        friend class ZayPanel;

    private:
        //! \brief 뷰의 영역객체
        class Element
        {
        public:
            typedef void (*GestureCB)(ZayView*, const Element*, GestureType, sint32, sint32);

        public:
            Element();
            ~Element();
            Element& operator=(const Element& rhs);

        public:
            PanelState GetState(void* touch) const;
            bool IsStateChanged(void* touch) const;

        public:
            sint32 m_updateid;
            String m_name;
            rect128 m_rect;
            float m_zoom;
            GestureCB m_cb;
            ZayPanel::SubGestureCB m_subcb;
            bool m_hoverpass;
            sint32 m_hoverid;
            mutable point64 m_saved_xy;

        private:
            mutable sint32 m_saved_updateid_for_state;
            mutable PanelState m_saved_state;
            mutable PanelState m_saved_state_old;
        };

    private:
        //! \brief 뷰의 터치관리
        class Touch
        {
        public:
            Touch();
            ~Touch();
            Touch& operator=(const Touch& rhs);

        private:
            class Cell
            {
            public:
                enum {Size = 128};
            public:
                sint32 m_updateid;
                sint32 m_validlength;
                Array<Element*> m_elements;
            public:
                Cell() {m_updateid = -1; m_validlength = 0;}
                ~Cell() {}
                Cell& operator=(const Cell& rhs)
                {
                    m_updateid = rhs.m_updateid;
                    m_validlength = rhs.m_validlength;
                    m_elements = rhs.m_elements;
                    return *this;
                }
            };
            typedef Array<Cell> CellRow;

        public:
            void ready(sint32 width, sint32 height);
            void update(chars uiname, float l, float t, float r, float b,
                float zoom, ZayPanel::SubGestureCB cb, bool hoverpass, bool* dirtytest = nullptr);
            const Element* get() const;
            const Element* get(chars uiname, sint32 lag) const;
            const Element& get(sint32 x, sint32 y) const;
            bool hovertest(sint32 x, sint32 y);

        private:
            Cell* getcell(sint32 x, sint32 y);
            const Cell* getcell_const(sint32 x, sint32 y) const;

        public:
            inline bool ishovered(sint32 hoverid) const {return (hoverid == m_hoverid);}
            inline sint32 hoverx() const {return m_hover_x;}
            inline sint32 hovery() const {return m_hover_y;}
            inline const Element* getfocus() const {return m_focus;}
            inline const Element* getpress() const {return m_press;}
            inline bool changefocus(const Element* element)
            {
                bool Result = (m_focus != element);
                m_focus = element;
                return Result;
            }
            inline bool changepress(const Element* element)
            {
                bool Result = (m_press != element);
                m_press = element;
                return Result;
            }
            inline const Element* changemoving(const Element* element, GestureType type)
            {
                const Element* Result = (m_moving == element)? nullptr : m_moving;
                m_moving = (type == GT_Moving)? element : nullptr;
                return Result;
            }
            inline const Element* changedropping(const Element* element, GestureType type)
            {
                const Element* Result = (m_dropping == element)? nullptr : m_dropping;
                m_dropping = (type == GT_Dropping)? element : nullptr;
                return Result;
            }

        private:
            static void OnGesture(ZayView* manager, const Element* data, GestureType type, sint32 x, sint32 y);
            static void OnSubGesture(ZayView* manager, const Element* data, GestureType type, sint32 x, sint32 y);

        private:
            sint32 m_updateid;
            sint32 m_hoverid;
            sint32 m_block_width;
            sint32 m_block_height;
            Element m_element;
            Map<Element> m_map;
            Array<CellRow> m_cell;
            const Element* m_focus;
            const Element* m_press;
            const Element* m_moving;
            const Element* m_dropping;
            sint32 m_hover_x;
            sint32 m_hover_y;
        };

    private:
        //! \brief 뷰의 함수관리
        class Function
        {
        public:
            Function();
            ~Function();

        public:
            bool m_isnative;
            String m_viewclass;
            ZayObject::CommandCB m_command;
            ZayObject::NotifyCB m_notify;
            ZayPanel::GestureCB m_gesture;
            ZayPanel::RenderCB m_render;
            ZayObject::BindCB m_bind;
            ZayObject::AllocCB m_alloc;
            ZayObject::FreeCB m_free;
        };

    protected:
        ZayView(chars viewclass = nullptr);
    public:
        ~ZayView() override;

    public:
        static View* Creator(chars viewclass);

    public:
        h_view SetView(h_view view) override;
        bool IsNative() override;
        void* GetClass() override;
        void SendNotify(chars sender, chars topic, id_share in, id_cloned_share* out) override;
        void SetCallback(UpdaterCB cb, payload data) override;
        void DirtyAllSurfaces() override;

    public:
        void OnCreate() override;
        bool OnCanQuit() override;
        void OnDestroy() override;
        void OnSize(sint32 w, sint32 h) override;
        void OnTick() override;
        void OnRender(sint32 width, sint32 height, float l, float t, float r, float b) override;
        void OnTouch(TouchType type, sint32 id, sint32 x, sint32 y) override;
        void OnKey(sint32 code, chars text, bool pressed) override;

    private:
        void _gesture(GestureType type, sint32 x, sint32 y);

    public:
        static autorun _makefunc(bool isnative, chars viewclass,
            ZayObject::CommandCB c, ZayObject::NotifyCB n, ZayPanel::GestureCB g, ZayPanel::RenderCB r,
            ZayObject::BindCB b, ZayObject::AllocCB a, ZayObject::FreeCB f);
        static Function* _accessfunc(chars viewclass, bool creatable);

    private:
        static const void* _finder(void* data, chars uiname);

    private:
        const Function* m_ref_func;
        const String m_viewclass;
        ZayObject* m_class;
        buffer m_touch;
        bool m_agreed_quit;
    };
}
