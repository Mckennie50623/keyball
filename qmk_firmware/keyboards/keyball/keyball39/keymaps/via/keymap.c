/*
Copyright 2022 @Yowkees
Copyright 2022 MURAOKA Taro (aka KoRoN, @kaoriya)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include QMK_KEYBOARD_H

#include "quantum.h"

// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
  // keymap for default
  [0] = LAYOUT_universal(
    KC_PSCR   , KC_F2     , KC_F3     , C(KC_C)  , C(KC_V),                     C(KC_Z)  , SSNP_VRT  , SSNP_HOR  , SSNP_FRE  , KC_DEL,
    KC_F4     , KC_F5     , KC_F6     , KC_TAB  , KC_LALT,                           SCRL_TO  , KC_BTN1  , KC_UP    , KC_BTN2  , A(KC_LEFT) ,
    KC_F7     , KC_F8     , KC_F9     , KC_LSFT  , KC_LGUI ,                           KC_HOME, KC_LEFT  , KC_DOWN  , KC_RGHT  , KC_END ,
    KC_F10   , KC_F11     , KC_F12   , KC_LCTL   , KC_ENT   , KC_BSPC  ,      TO(2)  , TO(1)  , _______  , _______  , _______  , RSFT_T(KC_ESC)
    ),
  [1] = LAYOUT_universal(
    KC_Q     , KC_W     , KC_E     , KC_R     , KC_T     ,                            KC_Y     , KC_U     , KC_I     , KC_O     , KC_P     ,
    KC_A     , KC_S     , KC_D     , KC_F     , KC_G     ,                            KC_H     , KC_J     , RCTL_T(KC_K) , RWIN_T(KC_L)    , KC_MINS  ,
    KC_Z     , KC_X     , KC_C     , KC_V     , KC_B     ,                            KC_N     , KC_M     , KC_COMM  , KC_DOT   , KC_SLSH  ,
    LCTL_T(KC_TAB), KC_LGUI, KC_LALT,KC_SPC  , KC_ENT   ,KC_BSPC,   LT(2,KC_LNG2)   , RSFT_T(KC_LNG1), _______, _______,_______, RSFT_T(KC_ESC)
  ),

  [2] = LAYOUT_universal(
    S(KC_6)    , KC_INT1     , KC_DLR    , KC_LBRC   , S(KC_2)  ,                            S(KC_RBRC)    ,   KC_1  , KC_2    , KC_3    ,  S(KC_BSLS)  ,
    S(KC_INT3)    , KC_SCLN     , KC_QUOT    , KC_EXLM   ,  S(KC_7)  ,                           S(KC_8), KC_4  , KC_5  , KC_6     , KC_0  ,
    KC_PERC    , KC_DQUO     , S(KC_SCLN)    , S(KC_INT1)    ,  KC_EQL,                            KC_RBRC  , KC_7   , KC_8  , KC_9  , KC_BSLS   ,
    KC_BSLS , KC_HASH  , S(KC_EQL)  , KC_SPC  , KC_ENT  , _______ ,       RCTL_T(KC_LNG2)   , TO(1)   , _______  , KC_RALT  , KC_RGUI  , S(KC_LBRC)
  ),

  [3] = LAYOUT_universal(
    RGB_TOG  , AML_TO   , AML_I50  , AML_D50  , _______  ,                            _______  , _______  , SSNP_HOR , SSNP_VRT , SSNP_FRE ,
    RGB_MOD  , RGB_HUI  , RGB_HUI  , RGB_VAI  , SCRL_DVI ,                            _______  , _______  , _______  , _______  , _______  ,
    RGB_RMOD , RGB_HUD  , RGB_SAD  , RGB_VAD  , SCRL_DVD ,                            CPI_D1K  , CPI_D100 , CPI_I100 , CPI_I1K  , KBC_SAVE ,
    QK_BOOT  , KBC_RST  , _______  , _______  , _______  , _______  ,      _______  , _______  , _______  , _______  , KBC_RST  , QK_BOOT
  ),
};
// clang-format on

layer_state_t layer_state_set_user(layer_state_t state) {
    // Auto enable scroll mode when the highest layer is 3
    keyball_set_scroll_mode(get_highest_layer(state) == 3);
    return state;
}

#ifdef OLED_ENABLE

#    include "lib/oledkit/oledkit.h"

void oledkit_render_info_user(void) {
    keyball_oled_render_keyinfo();
    keyball_oled_render_ballinfo();
    keyball_oled_render_layerinfo();
}
#endif

const uint16_t PROGMEM combo1[] = {KC_LCTL, TO(2) , COMBO_END};
const uint16_t PROGMEM combo2[] = {S(KC_INT3), S(KC_BSLS), COMBO_END};
const uint16_t PROGMEM combo3[] = {KC_C, KC_LGUI, COMBO_END};
const uint16_t PROGMEM combo4[] = {SSNP_HOR, KC_LCTL, COMBO_END};
const uint16_t PROGMEM combo5[] = {SSNP_FRE, KC_LCTL, COMBO_END};
combo_t key_combos[] = {
    COMBO(combo1, TO(3)),
    COMBO(combo2, KC_BSLS),
    COMBO(combo3, G(KC_C)),
    COMBO(combo4, C(KC_SCLN)),
    COMBO(combo5, C(KC_QUOT)),
};

#include "timer.h"

enum layer_names { _BASE = 0, _ALPHA = 1, _SYM = 2, _ADJ = 3 };

static uint16_t last_move_timer = 0;
static uint16_t lang2_timer = 0;
static bool pending_lang2 = false;
static uint8_t lang2_count = 0;  // キー送信回数をカウント
#define MOVE_DEBOUNCE_MS 10
#define LANG2_DELAY_MS 30  // 8msから30msに延長（OS/IMEが確実に受け取れるように）
#define LANG2_REPEAT_DELAY_MS 10  // 2回目の送信までの間隔

#define MOTION_THRESHOLD 6

static inline uint8_t motion_amount(const report_mouse_t *m) {
    // スクロールを無視したいなら h/v を入れない（下はx,yのみ）
    int16_t s = (m->x >= 0 ? m->x : -m->x) + (m->y >= 0 ? m->y : -m->y);
    // スクロールも判定に含めたい場合は下2行も加える:
    // s += (m->h >= 0 ? m->h : -m->h);
    // s += (m->v >= 0 ? m->v : -m->v);
    return (uint8_t)s;
}

// 括弧の自動ペアリング処理
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (record->event.pressed) {
        // キーコードを直接チェック
        switch (keycode) {
            case S(KC_8):  // (
                tap_code16(S(KC_8));  // (
                tap_code16(S(KC_9));  // )
                tap_code(KC_LEFT);  // カーソルを括弧の間に戻す
                return false;
                
            case S(KC_9):  // ) - 閉じ括弧が押された場合は何もしない（通常の動作）
                return true;
                
            case KC_RBRC:  // [
                tap_code(KC_RBRC);  // [
                tap_code(KC_BSLS);  // ]
                tap_code(KC_LEFT);  // カーソルを括弧の間に戻す
                return false;
                
            case KC_BSLS:  // ] - 閉じ括弧が押された場合は何もしない（通常の動作）
                return true;
                
            case S(KC_RBRC):  // { - シフト+[が押された場合
                tap_code16(S(KC_RBRC));  // {
                tap_code16(S(KC_BSLS));  // }
                tap_code(KC_LEFT);  // カーソルを括弧の間に戻す
                return false;
                
            case S(KC_BSLS):  // } - 閉じ括弧が押された場合は何もしない（通常の動作）
                return true;
        }
        
        // QK_MODS形式のキーコードもチェック（S(KC_LBRC)などがこの形式で来る可能性がある）
        if (keycode >= QK_MODS && keycode <= QK_MODS_MAX) {
            uint16_t base_keycode = keycode & 0xFF;
            bool shift_pressed = (keycode & QK_LSFT) || (keycode & QK_RSFT);
            
            if (shift_pressed) {
                switch (base_keycode) {
                    case KC_LBRC:  // { - S(KC_LBRC)が押された場合
                        tap_code16(S(KC_LBRC));  // {
                        tap_code16(S(KC_RBRC));  // }
                        tap_code(KC_LEFT);  // カーソルを括弧の間に戻す
                        return false;
                        
                    case KC_RBRC:  // } - 閉じ括弧が押された場合は何もしない（通常の動作）
                        return true;
                }
            }
        }
    }
    return true;  // 他のキーは通常通り処理
}

report_mouse_t pointing_device_task_user(report_mouse_t mouse_report) {
    // 保留中のキー送信を処理（非同期で軽量化、マウスレポートをブロックしない）
    if (pending_lang2) {
        uint16_t elapsed = timer_elapsed(lang2_timer);
        if (lang2_count == 0 && elapsed > LANG2_DELAY_MS) {
            // 1回目の送信
            tap_code(KC_LNG2);  // 半角（英数）に切り替え
            lang2_count = 1;
            lang2_timer = timer_read();  // タイマーをリセット
        } else if (lang2_count == 1 && elapsed > LANG2_REPEAT_DELAY_MS) {
            // 2回目の送信（確実性を高めるため）
            tap_code(KC_LNG2);
            pending_lang2 = false;
            lang2_count = 0;
        }
    }
    
    if (mouse_report.x || mouse_report.y || mouse_report.h || mouse_report.v) {
        uint8_t motion = motion_amount(&mouse_report);
        if (motion >= MOTION_THRESHOLD) {
            if (timer_elapsed(last_move_timer) > MOVE_DEBOUNCE_MS) {
                if (get_highest_layer(layer_state) != _BASE) {
                    clear_oneshot_layer_state(0);
                    clear_oneshot_mods();
                    layer_clear();
                    layer_move(_BASE);
                    // キー送信を遅延させて非同期処理（重い処理を避けてProMicroの負荷を軽減）
                    pending_lang2 = true;
                    lang2_count = 0;  // カウンターをリセット
                    lang2_timer = timer_read();
                    last_move_timer = timer_read();
                }
            }
        }
    }
    return mouse_report;
}