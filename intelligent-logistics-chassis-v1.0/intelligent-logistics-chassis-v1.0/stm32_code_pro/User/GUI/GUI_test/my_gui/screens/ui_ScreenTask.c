// This file was generated by SquareLine Studio
// SquareLine Studio version: SquareLine Studio 1.3.2
// LVGL version: 8.3.6
// Project name: SquareLine_Project

#include "../ui.h"

void ui_ScreenTask_screen_init(void)
{
ui_ScreenTask = lv_obj_create(NULL);
lv_obj_clear_flag( ui_ScreenTask, LV_OBJ_FLAG_SCROLLABLE );    /// Flags

ui_ButtonReturn3 = lv_btn_create(ui_ScreenTask);
lv_obj_set_width( ui_ButtonReturn3, lv_pct(15));
lv_obj_set_height( ui_ButtonReturn3, lv_pct(10));
lv_obj_add_flag( ui_ButtonReturn3, LV_OBJ_FLAG_SCROLL_ON_FOCUS );   /// Flags
lv_obj_clear_flag( ui_ButtonReturn3, LV_OBJ_FLAG_SCROLLABLE );    /// Flags
lv_obj_set_style_bg_color(ui_ButtonReturn3, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT );
lv_obj_set_style_bg_opa(ui_ButtonReturn3, 255, LV_PART_MAIN| LV_STATE_DEFAULT);

ui_LabelReturn3 = lv_label_create(ui_ButtonReturn3);
lv_obj_set_width( ui_LabelReturn3, LV_SIZE_CONTENT);  /// 1
lv_obj_set_height( ui_LabelReturn3, LV_SIZE_CONTENT);   /// 1
lv_obj_set_align( ui_LabelReturn3, LV_ALIGN_CENTER );
lv_label_set_text(ui_LabelReturn3,"<--");
lv_obj_set_style_text_color(ui_LabelReturn3, lv_color_hex(0x100F0F), LV_PART_MAIN | LV_STATE_DEFAULT );
lv_obj_set_style_text_opa(ui_LabelReturn3, 255, LV_PART_MAIN| LV_STATE_DEFAULT);

ui_Panel2 = lv_obj_create(ui_ScreenTask);
lv_obj_set_width( ui_Panel2, lv_pct(98));
lv_obj_set_height( ui_Panel2, lv_pct(50));
lv_obj_set_x( ui_Panel2, 0 );
lv_obj_set_y( ui_Panel2, lv_pct(-10) );
lv_obj_set_align( ui_Panel2, LV_ALIGN_CENTER );
lv_obj_clear_flag( ui_Panel2, LV_OBJ_FLAG_SCROLLABLE );    /// Flags

ui_LabelTaskNum = lv_label_create(ui_Panel2);
lv_obj_set_width( ui_LabelTaskNum, LV_SIZE_CONTENT);  /// 1
lv_obj_set_height( ui_LabelTaskNum, LV_SIZE_CONTENT);   /// 1
lv_obj_set_align( ui_LabelTaskNum, LV_ALIGN_CENTER );
lv_label_set_text(ui_LabelTaskNum,"---+---");
lv_obj_set_style_text_font(ui_LabelTaskNum, &ui_font_bigNum, LV_PART_MAIN| LV_STATE_DEFAULT);

ui_ButtonTaskRun = lv_btn_create(ui_ScreenTask);
lv_obj_set_width( ui_ButtonTaskRun, 100);
lv_obj_set_height( ui_ButtonTaskRun, 50);
lv_obj_set_x( ui_ButtonTaskRun, 0 );
lv_obj_set_y( ui_ButtonTaskRun, lv_pct(-5) );
lv_obj_set_align( ui_ButtonTaskRun, LV_ALIGN_BOTTOM_MID );
lv_obj_add_flag( ui_ButtonTaskRun, LV_OBJ_FLAG_SCROLL_ON_FOCUS );   /// Flags
lv_obj_clear_flag( ui_ButtonTaskRun, LV_OBJ_FLAG_SCROLLABLE );    /// Flags
lv_obj_set_style_bg_color(ui_ButtonTaskRun, lv_color_hex(0xED1818), LV_PART_MAIN | LV_STATE_PRESSED );
lv_obj_set_style_bg_opa(ui_ButtonTaskRun, 255, LV_PART_MAIN| LV_STATE_PRESSED);

ui_Label4 = lv_label_create(ui_ButtonTaskRun);
lv_obj_set_width( ui_Label4, LV_SIZE_CONTENT);  /// 1
lv_obj_set_height( ui_Label4, LV_SIZE_CONTENT);   /// 1
lv_obj_set_align( ui_Label4, LV_ALIGN_CENTER );
lv_label_set_text(ui_Label4,"Start");

ui_LabelFinishTime = lv_label_create(ui_ScreenTask);
lv_obj_set_width( ui_LabelFinishTime, LV_SIZE_CONTENT);  /// 1
lv_obj_set_height( ui_LabelFinishTime, LV_SIZE_CONTENT);   /// 1
lv_obj_set_x( ui_LabelFinishTime, 0 );
lv_obj_set_y( ui_LabelFinishTime, -99 );
lv_obj_set_align( ui_LabelFinishTime, LV_ALIGN_CENTER );
lv_label_set_text(ui_LabelFinishTime,"finish: 4:30");
lv_obj_add_flag( ui_LabelFinishTime, LV_OBJ_FLAG_HIDDEN );   /// Flags

lv_obj_add_event_cb(ui_ButtonReturn3, ui_event_ButtonReturn3, LV_EVENT_ALL, NULL);
lv_obj_add_event_cb(ui_ButtonTaskRun, ui_event_ButtonTaskRun, LV_EVENT_ALL, NULL);

}