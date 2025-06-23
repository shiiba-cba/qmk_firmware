#include QMK_KEYBOARD_H

enum layer_names {
    _BASE,
    _SCROLL
};

enum custom_keycodes {
    CPI_TGL = SAFE_RANGE
};

/* ───── キーマップ ───── */
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    /* 基本レイヤー */
    [_BASE] = LAYOUT_direct(
        KC_BTN1, KC_BTN2, KC_BTN3, KC_BTN4, KC_BTN5,
        MO(_SCROLL)              /* ← DRAG_SCROLL (GP29) */
    ),

    /* Drag-Scroll レイヤー (MO で一時起動) */
    [_SCROLL] = LAYOUT_direct(
        CPI_TGL, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS                      /* DRAG_SCROLL 自身は何でも可 */
    ),
};

/* ───── 変数 ───── */
static bool drag_scroll = false;
static bool cpi_low     = false;

static const uint16_t CPI_DEF = PMW33XX_CPI;          // 600dpi など
static const uint16_t CPI_ALT = CPI_DEF / 3;          // 200dpi
static const uint16_t CPI_SCROLL = 50;                // Drag-Scroll 中の固定値

/* ───── キー処理 ───── */
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        /* 左ボタン → CPI_TGL に置き換え済みなのでクリックを送らない */
        case CPI_TGL:
            if (record->event.pressed) {
                cpi_low = !cpi_low;                      // 切り替え
            }
            return false;                               // OS へは送らない
    }
    return true;
}

/* ───── レイヤー出入りで CPI と状態を管理 ───── */
layer_state_t layer_state_set_user(layer_state_t state) {
    bool now_scroll = layer_state_cmp(state, _SCROLL);
    if (now_scroll != drag_scroll) {                    // 状態が変わったときだけ
        drag_scroll = now_scroll;
        pointing_device_set_cpi(
            drag_scroll ? CPI_SCROLL                    // 入った瞬間 50dpi
                        : (cpi_low ? CPI_ALT : CPI_DEF) // 抜けたら通常/低感度
        );
    }
    return state;
}

/* ───── ポインタ → スクロール変換 ───── */
report_mouse_t pointing_device_task_user(report_mouse_t m) {
    if (drag_scroll) {
        m.h =  m.x;
        m.v = -m.y;
        m.x = m.y = 0;
    }
    return m;
}

