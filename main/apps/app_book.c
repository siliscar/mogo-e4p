/*
 * SPDX-FileCopyrightText: 2026 mindreset
 * SPDX-License-Identifier: Apache-2.0
 *
 * 图书书架、阅读、目录与页内进度；文件解析和排版由book模块负责。
 * Book shelf, reader, TOC and progress; book modules own parsing and pagination.
 *
 * 用户修订：阅读设置采用已确认的半屏双列排版；自动全刷与翻页效果点按轮换，保留按键控制二级页。
 * User revision: use the approved compact two-column reading sheet; tap cycles refresh/effect and key controls retain their secondary page.
 * 冻结：Phase4b统一手势入口并接管三键为上页/工具条/下页；工具栏开关只对底部带做 GL16 差分。屏幕翻页在抬起提交，不画按下态。
 * 用户修订：图文混排原样采用 PR9 的 book_layout；调用侧只适配图片缓存与上下文生命周期。
 * 晃动默认关；横向左晃上一页、右晃下一页，触摸与回弹不触发；离页恢复加速度配置并休眠。render只绘图。
 * 预渲染与同步推屏重叠、按行可停；推屏返回前交还字体与章节资源。
 * 普通翻页优先刷新正文，再更新页脚和有变化的顶部；手动与周期清残影共用整屏 GC16。
 * Ordinary turns present the body before the footer and changed status; manual and periodic cleanup share whole-screen GC16.
 * 用户 2026-10-30 修订：时间与电量随翻页更新，静置不单刷顶部；为缩短顶底栏刷新，顶部恢复默认 GL16，普通页脚恢复短 DU，换章与插图页脚保留默认 GL16 清理，正文仍优先。
 * User revision 2026-10-30: update time/power with page turns, without idle top-band output;
 * shorten chrome output with default GL16 status and short-DU ordinary footers, retain default GL16
 * cleanup for chapter/illustration footers, and keep body-first presentation.
 * 用户修订：正文单击按左右各半或上1/3下2/3翻页，不受按键配置影响；全屏由工具栏「全屏」按钮与「切换全屏」按键动作切换。
 * 用户 2026-10-17 修订：移除按键控制的两套推荐设置，新增中键双击动作，默认不操作；绑定后单击等待 340ms 双击窗口。
 * User revision 2026-10-17: drop the two key presets and add a configurable middle double press, defaulting to no action; once bound, a single click waits through the 340 ms window.
 * 用户 2026-10-21 修订：按键控制示意图按本板实体顺序摆成中、左、右（中键最左），中键双击下新增音量键翻页开关。
 * User revision 2026-10-21: the key sheet follows the board's physical key order (middle, left, right), and a volume-key page-turn switch sits under the middle double press.
 * 用户 2026-10-22 修订：中键双击行与中键长按同高；音量键翻页上方新增上键单击，列表超出视口一行时按行滚动。
 * User revision 2026-10-22: the double-press row matches the hold row, a new up-key single press sits above the
 * volume-key switch, and the list scrolls a whole row at a time because it outgrows the viewport by one row.
 * 用户 2026-10-22 修订：按键控制的动作列表新增「前光开关」，可绑给任一单键、长按、双击或上键。
 * User revision 2026-10-22: the key-control action list gains a frontlight toggle that can be bound
 * to any short key, the middle hold, the middle double press or the up key.
 * 用户 2026-10-27 修订：阅读背景加水波纹时插图页残留上一页字影；插图页改用独立波形表（深色对
 * 保留前导擦白、相等对保持），背景派生表只留给纯文字页。
 * User revision 2026-10-27: with a reading background the ripple left the previous page's glyphs on
 * illustration pages; those frames now take their own waveform (dark pairs keep the leading erase,
 * equal pairs hold), and the background derivative serves text pages only.
 * 三键单击、中键长按与中键双击可自定义；中键长按默认返回主页，阅读页不画右下角菜单图标。
 * 用户授权基础管理：长按书架先看完整详情，清进度与删文件分别确认；失败保留待重试记录，不自动回收其他书进度。
 * 用户2026-11-05修订：分类仅保存书架归属，不搬文件；新建后立即选书，归类勾选跨搜索/筛选保留，避免反复重选。
 * 单本管理仍为书架弹窗；原文件批量管理保留其筛选、搜索及重扫清除勾选的语义。
 * 失败进度仅按变更路径失效；删除后的清理重试保留到本次开机结束，不随切页释放。
 * 卡失效时先保存进度并关闭阅读资源，再由主循环回退字体；禁止自动续读失效挂载。
 * 用户最新修订：书架只保留深色书轨与亚克力，每页九本；收录导入及读过的书，移出仅隐藏，再读重新上架。
 * 底栏保留首页/书架/文件/设置，设置直接进入设置页；用户2026-11-05确认多选时以宽松批量操作栏替换导航，取消后恢复，解决误触与拥挤。
 * 用户修订：长按图书可用本机拼音输入编辑书名；阅读时长与翻页真实记录，供票根锁屏使用。
 * 用户最新修订：书名编辑可点选插入位置并用左右键微调，支持在文字中间插入和删除。
 * 用户授权新输入法：书名和搜索共用九宫格/全键盘、中英文及离线词语候选，不改正文排版。
 * 用户修订：首页、书架、文件、设置四栏导航；切换界面采用 GL16，章节首页单独排标题。
 * 用户修订：EPUB 章节首页以书内目录标题为准，正文题头仅在相同时折叠，避免引言误判。
 * 用户修订：阅读进度条无外伸刻度并使用圆角；继续阅读区显示最近书籍封面。
 * 用户修订：书架整页读取成品缓存后一次显示；首次生成有界分批处理，但整页封面一起显现，不逐本推屏。
 * User revision: read the page's finished caches before its first frame; cold generation is bounded, but cover artwork is revealed together.
 * 用户修订：字号和间距重排只保存最终进度，字号使用现有闲置灰阶整理。
 * 用户修订：阅读设置滑杆可拖动并在松手后重排；字体选择可纵向翻页；统计入口显示真实明细与近30天数据。
 * 用户修复：间距滑杆点按和拖动共用完整档位，字间距每次1px，段距按显示档位递进。
 * User fix: spacing taps and drags share the full ladders; tracking steps by 1px and paragraph spacing follows the displayed stops.
 * 用户修订：前后均为纯文字的普通翻页采用柔和 GL16 差分，仅白底灰边提前少量回浅，不改字形或灰度目标；插图、混排、周期全刷及水波纹保留原规则。
 * User revision: soften ordinary pure-text GL16 turns by advancing a few white-to-gray settling pulses; preserve glyph targets, images, mixed pages, cleanup and ripple.
 * 用户 2026-10-16 修订：周期清残影改走整屏 GC16，原从白重绘的黑闪次数过多。
 * User revision 2026-10-16: periodic cleanup now uses whole-screen GC16; the from-white redraw flashed too many times.
 * 用户修订：目录由独立模块整页绘制与命中；目录标题清理换行并限制为单行，翻页不再沿用书架的局部刷新。
 * 用户修订：书架封面抽出与取消仅驱动变化像素，保持灰阶，不在点按时强制清屏。
 * 用户修订：列表封面上弹8px，松手先回位再开书；只重画两位置的并集，复用缓存，不触碰文字/边框/分隔线。快刷用跟随DU，其他模式保留灰阶差分。
 * User revision: list covers bounce up 8px and settle before opening; repaint only the position union using cached artwork, preserving text, frames and separators. Fast uses FOLLOW DU; other modes retain gray differentials.
 * 用户确认：列表封面缩至120×156，保留原中心点与右侧文字位置，等比填充并居中裁切。
 * User approval: list covers use 120×156 boxes, retaining their center and right-side text positions with aspect fill and centered cropping.
 * 用户修订：快刷亚克力采用规则一像素棋盘格，与已缓存封面合并输出，不注册灰区或等待框架先显示；未缓存封面继续有界延后解码。普通及水波纹保留灰阶，切页/锁屏取消旧目标。
 * 用户修订：书架读取最多当前整页缓存；未缓存封面限时准备，整组显现后才确认待刷标记，文件间检查导航，不新增整页缓存。
 * 用户修订：阅读背景先列pictures图片、忽略隐藏文件，点开预览上下切换，返回目录；背景按整屏坐标固定，不随文字边距或全屏状态缩放。
 * User revision: fast acrylic uses a regular one-pixel checkerboard in the same output as cached covers, without gray registration or a furniture-first wait; uncached covers retain bounded lazy decoding. Ordinary/water retain grays; page/lock changes cancel stale targets.
 * 用户要求采用PR17的快速水波纹，并最新指定8条带、每拍11090us；每次显式设置，结束恢复先前节拍，避免影响主页动画。全刷、插图及失败恢复保留原优先级。
 * User requests PR17 fast ripple with the latest 8 bands at 11090us; explicitly select pacing per turn and restore it afterward so main animations remain independent. Retain cleanup, image and recovery priority.
 * 用户增加管理/导入按压态：复用现有抬起确认流程，按下灰底加深并下沉2px，只推按钮区域。
 * User adds management/import feedback: retain release-confirmed actions, darken and depress controls 2px on press, presenting only their region.
 * 用户修订：快刷封面保留轻量减黑并按最新要求略增黑点密度，在PSRAM缓存最多九张1位缩略图（约40KiB）；进度未改变时不重复丢弃封面。缓存只画封面，收藏与界面文字实时绘制，开书或卡失效时释放。
 * Frozen: Phase4b uses the shared gesture entry and owns previous/tools/next keys; a tools toggle runs a GL16 differential over the bottom band. Screen turns commit on release without pressed decoration.
 * User revision: adopt PR9's book_layout unchanged; callers only adapt bitmap caching and context lifetimes.
 * Authorized keyboard revision: titles and search share T9/QWERTY, bilingual offline phrase candidates; body layout stays intact.
 * 用户修订：输入不重画题头、封面或正文，只刷新输入变化区域，退出后释放查字缓存。
 * 用户修订：新开书沿用记住的全屏偏好（默认全屏）；翻页收起工具栏与滑块但不改变全屏状态；重排以已选目标页的文字位置为锚，不跳过未读文字。
 * User revision: new books follow the remembered full-screen preference (default on); turns dismiss the toolbar and sliders but keep the current full-screen state; reflow anchors the selected destination before changing geometry so unread text is not skipped.
 * 用户修订：字重用横向减/加胶囊，每次微调1/16像素覆盖率；保持字形比例/字宽，不加载额外整份字库。
 * User revision: a horizontal minus/plus pill adjusts coverage by 1/16 pixel per tap, preserving advances/proportions without another font.
 * 用户确认：字体/字重与书内字体/排版分别并排，移除阅读背景入口；字号和阅读线继续可调。
 * User approval: pair face/weight and embedded-face/typography, removing the background entry while keeping size and reading rules adjustable.
 * 用户 2026-10-25 修订：阅读背景入口迁至设置页「个性化功能」，本文件不再保留选择面板；背景的解码与绘制路径不变。
 * User revision of 2026-10-25: the reading-background entry moves to Settings personalization, so this file keeps no picker panel; decoding and painting paths stay unchanged.
 * 用户 2026-10-25 续定：背景下恢复下一页预渲染与水波纹；水波纹门改用「稳定文字帧」（含背景），
 * 文字软化波形仍只认纯文字帧，插图页与失败恢复维持原优先级。
 * User follow-up of 2026-10-25: backgrounds re-enable next-page prefetch and the water
 * ripple; the ripple gate now uses a stable text frame (backgrounds included) while the
 * softened text waveform stays pure-text, with image pages and failure recovery unchanged.
 * 用户增加阅读抗锯齿开关，默认开；书架整页封面合并显示；正文底部上滑固定呼出工具栏，不受翻页/按键设置影响。
 * User adds default-on reading AA and grouped shelf artwork; upward swipes from the body bottom always open tools independently of tap/key modes.
 * 用户修订：底部上滑固定呼出工具栏，不随按键及翻页模式变化；取消短按或长按必须保留工具栏的限制。
 * User revision: bottom-edge swipes always open tools independently of key/tap modes; short and hold actions no longer require a toolbar mapping.
 * User revision: input refreshes changed regions without repainting headers, covers or body; release lookup caches on exit.
 * Shake is off by default: left/right impulses turn back/forward, suppressing touch and rebound; exit restores the sensor and sleeps it. Render only paints.
 * Prefetch overlaps synchronous output and stops between rows; no font/chapter resources survive presentation.
 * Ordinary turns refresh only body and footer; manual and periodic ghost cleanup share one whole-screen GC16 output.
 * User revision: single body taps use halves or upper 1/3/lower 2/3 independently of key mappings; full screen is toggled from the toolbar button or a full-screen key action, and center double taps are gone.
 * Short key actions are configurable with two presets; middle holds are configurable and default to home. The reader has no bottom-right menu icon.
 * User-authorized management shows full details before separate clear/delete confirmations; retain failed saves for retry without pruning other books.
 * User revision2026-11-05: logical categories never move files; create then pick books, retaining classification selections across search/filters to avoid repeated selection.
 * Single-book actions remain a shelf dialog; existing file-batch filters, applied searches and rescans still clear selections.
 * Invalidate failed progress only for changed paths; retain deletion cleanup retries across page exits for this boot.
 * Lost media saves progress and closes reader resources before global font fallback; never auto-resume an invalid mount.
 * Latest user revision: the two retained shelf styles hold nine imported or read books; retired styles restore as acrylic, and removal hides until reread.
 * The fourth tab opens Settings directly; user approval2026-11-05 replaces navigation with roomy batch actions while selecting and restores it on cancel to prevent cramped targets.
 * User revision: book details lead to an on-device Pinyin title editor; measured reading time and turns feed the ticket lock face.
 * Latest user revision: the title editor can place and move an insertion caret for edits in the middle of text.
 * 用户统一修订：书名与搜索采用共用常亮光标；点文字或箭头定位，按系统字体测量。
 * User-wide revision: title/search share a steady caret, text/arrow positioning and system-font measurement.
 * User revision: home, shelf, files and settings have four-tab navigation; view changes use GL16 and chapter starts have a title lead.
 * User revision: EPUB chapter leads use navigation titles; body headings are folded only when matching, preventing front matter from being mislabeled.
 * User revision: the reader bar is rounded without protruding ticks; continue reading displays the latest cover.
 * User revision: the shelf retains its title layout, reads a complete cached page before painting, and reveals cold artwork only after the page is prepared.
 * User revision: size and spacing reflow saves final progress only; size uses the existing idle grayscale settle.
 * User revision: reader sliders drag and reflow on release; font selection pages vertically; statistics entries show real details and recent-30-day data.
 * User revision: pure-text turns use softened gray-edge differential GL16; images, mixed pages and water turns retain their policies, while periodic cleanup uses whole-screen GC16.
 * User revision: a standalone module owns full-page TOC rendering and hit testing; normalized single-line titles cannot leak into another row.
 * User revision: cover lift and cancellation drive changed pixels in grayscale, without forced cleanup during a tap.
 * 用户修订：快刷仅常驻黑白缩略图；灰阶解码缓冲转好即释放，灰阶模式按需重建，收藏白边单独记录。
 * User revision: fast mode retains only packed monochrome thumbnails, frees gray decoding buffers immediately after conversion, rebuilds gray on demand, and stores favorite-edge contrast separately.
 * User revision: fast covers retain a light white lift with slightly denser blacks per the latest request and cache at most nine 1-bit thumbnails in PSRAM (about 40 KiB); unchanged progress no longer discards covers. Cache artwork only, draw favorites and UI text live, and release on book opening or media loss.
 * 用户最新修订：阅读书名与章节标题复用当前阅读字体，以原32/48px直接光栅化；首页另用其活动系统字体，不同时加载两份字库。
 * Latest user revision: book/chapter titles reuse the current reader face at the original 32/48px; Home uses its active system face, never retaining both fonts.
 * 用户2026-11-05修订：题头下拉切换独立分类，旧子目录仅首次迁移；各类别/布局记住页码，普通浏览主体不变，多选单独留出筛选与操作带。
 * User revision2026-11-05: title choices are independent categories with one-time legacy-folder migration; retain per-category/layout pages and browsing geometry, reserving separate selection/filter bands while selecting.
 * 用户2026-11-06修订：普通书架移除题头下重复数量，搜索、选择、管理、导入同排等距；分类标题与按钮命中互不重叠，保留底部分页数量。
 * User revision2026-11-06: remove the duplicate header count, evenly space search/select/manage/import in one row, isolate title hits and retain the pager count.
 * 用户修订：页眉去掉返回钮与收藏图标，书名左置；右侧改画状态栏同款时间与电池；三键新增「返回」动作（按来路回退），收藏开关移到书架详情页。
 * User revision: drop the header back/favorite controls and left-align the title; the right side draws the status-bar clock and battery; key actions gain "back" (undoing the entry path) and the favorite toggle moves to the shelf details panel.
 * 用户修订：工具栏展开时点按正文只收起工具栏，不再顺带翻页；滑动、按键与电源键仍翻页。
 * User revision: a body tap with the tools bar open only dismisses it; swipes, keys and the power key still turn pages.
 * 用户 2026-10-31 修订：页眉页脚各分左中右三槽，内容在阅读设置内自选；默认页眉变为
 * 书名/时间/电量图标，页脚保持章节名/空/页码·进度。页眉合成文本指纹随翻页对比，
 * 内容变化才推顶部差分带，静置仍不单刷。
 * User revision of 2026-10-31: each bar splits into left/center/right slots picked in the reading
 * sheet; the default header becomes title/clock/battery icon while the footer keeps
 * chapter/blank/page·progress. The composed header fingerprint is compared per turn and the top
 * diff band only presents when it changed; idle stays silent.
 * 用户 2026-11-02 修订：页眉页脚高度独立调节，默认几何不变；正文、信息栏绘制与刷新边界同步重排，全屏不受影响。
 * User revision 2026-11-02: independent bar heights retain default geometry; body, chrome and refresh bounds reflow together without changing full-screen geometry.
 * 用户 2026-10-21 修订：工具栏开关恢复为带内 GL16 差分；此前改 GC16 只因实心进度条擦除后留轮廓，
 * 差分脏区膨胀已补推边缘邻行/邻列，不再需要整块全像素驱动。
 * User revision 2026-10-21: the tools toggle returns to a band GL16 differential. GC16 was only
 * needed while erasing the solid progress bar left an outline; dirty-mask dilation now re-drives
 * the neighbouring rows/columns, so the full-pixel pass is no longer required.
 */
#include "../diagnostics/reader_perf.h"
#include "../diagnostics/reader_memory.h"
#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "app.h"
#include "touch_diag.h"
#include "app_content_open.h"
#include "boot_state.h"
#include "app_registry.h"
#include "ble_page_turner.h"
#include "book_layout.h"
#include "reader_chrome.h"
#include "book_cover.h"
#include "book_shelf_covers.h"
#include "book_lock_collage.h"
#include "book_epub.h"
#include "book_policy.h"
#include "book_file_path.h"
#include "book_progress.h"
#include "book_bookmark_format.h"
#include "book_category_ops.h"
#include "book_category_ui.h"
#include "book_selection_ui.h"
#include "usb_storage.h"
#include "book_font_size.h"
#include "book_title.h"
#include "book_ticket.h"
#include "reading_journal.h"
#include "book_excerpt.h"
#include "book_toc.h"
#include "app_font_context.h"
#include "book_source.h"
#include "reader_session.h"
#include "reader_turn.h"
#include "reader_reflow.h"
#include "reader_open.h"
#include "reader_session_layout.h"
#include "reader_chapter.h"
#include "reader_prefetch.h"
#include "reader_present.h"
#include "book_store.h"
#include "book_shelf_category_state.h"
#include "book_shelf_categories.h"
#include "book_shelf_scan.h"
#include "book_shelf_manage.h"
#include "book_shelf_model.h"
#include "display.h"
#include "display_refresh.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "frontlight.h"
#include "device_power.h"
#include "nvs.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "board_init.h"
#include "board_hardware.h"
#include "board_sd.h"
#include "wifi_transfer.h"
#include "book_search.h"
#include "settings.h"
#include "book/book_reader_input.h"
#include "ttf_font.h"
#include "ui_kit.h"
#include "ui_reader_heading.h"
#include "ui_reader_settings.h"
#include "ui_text_input.h"
#include "ui_keyboard.h"
#include "ui_image_dither.h"
#include "ui_reader_background.h"
#include "ui_gesture.h"
#include "ui_menu.h"
#include "ui_nav.h"
#include "ui_click_feedback.h"
#include "app_transfer_mode.h"

// 阅读工具栏图标盒边长；行内图标在 1096..1216 的工具条里以 1138 为中心。
// Reader toolbar icon box; the row centres on y=1138 inside the 1096..1216 bar.
#define READER_TOOL_ICON_PX 40
// 二级面板返回键里的箭头边长；圆底半径 24，原来的箭头是 20 像素高。
// Chevron box inside a secondary panel's back control; the old arrow was 20 px tall.
#define READER_SHEET_BACK_PX 52

#define BOOK_ROWS 13
#define BOOK_GRID_ROWS 9
#define BOOK_BULK_ROWS 6
#define BOOK_PX_MIN BOOK_READER_PX_MIN
#define BOOK_PX_MAX BOOK_READER_PX_MAX
#define BOOK_PX_STEP 1
#define BOOK_MARGIN_MIN 0
#define BOOK_MARGIN_MAX 96
#define BOOK_MARGIN_STEP 6
#define BOOK_MARGIN_CHOICES ((BOOK_MARGIN_MAX - BOOK_MARGIN_MIN) / BOOK_MARGIN_STEP + 1)
// 四向边距的顺序：状态、滑杆索引和持久化都按这个次序。/ Four margins share one order for state, rails and persistence.
enum { MARGIN_LEFT, MARGIN_RIGHT, MARGIN_TOP, MARGIN_BOTTOM, MARGIN_COUNT };
#define BOOK_TOOL_COUNT 6
#define BOOK_FONT_PAGE 9
#define BOOKMARK_ROWS 6

typedef enum { SHELF, READING, TOC, MANAGE, BULK, IMPORT, SEARCH, EDIT, CATEGORIES } book_view_t;
typedef enum {
    READER_PANEL_NONE,
    READER_PANEL_TOOLS,
    READER_PANEL_FONT_SETTINGS,
    READER_PANEL_LAYOUT_SETTINGS,
    READER_PANEL_WEIGHT_SETTINGS,
    READER_PANEL_RULE_SETTINGS,
    READER_PANEL_FONT_PICKER,
    READER_PANEL_STATS,
    READER_PANEL_BOOKMARKS,
    READER_PANEL_STATS_RECENT,
    READER_PANEL_REFRESH_SETTINGS,
    READER_PANEL_TURN_SETTINGS,
    READER_PANEL_KEY_SETTINGS,
    READER_PANEL_KEY_PICKER,
    READER_PANEL_CHROME_SETTINGS,
    READER_PANEL_CHROME_PICKER,
    READER_PANEL_TITLE_STYLE,
    READER_PANEL_NOTE,
} reader_panel_t;
typedef book_shelf_entry_t shelf_entry_t;

static const char* TAG = "book";
static book_view_t s_view;
static int s_presented_view = -1;
static bool s_reader_image_refresh_pending;
static bool s_reader_turn_pending;
// 只记录成功推屏的纯正文基准，避免跨章或退出插图时误用文字波形。
// Track only successfully presented text-only bodies, protecting chapter and image transitions.
static bool s_reader_text_frame;
// 稳定文字帧再放行阅读背景：水波纹的原厂表按灰阶对迁移，背景不需要排除；
// 文字软化波形仍只认纯文字的 s_reader_text_frame。
// A stable text frame additionally admits reading backgrounds: the ripple's vendor table
// migrates per gray pair, so backgrounds need no exclusion; the softened text waveform
// still requires the pure-text s_reader_text_frame.
static bool s_reader_stable_frame;
// 工具栏开关只改底部一条带；这一帧只对带内做 GL16 差分，脏区膨胀负责擦掉进度条边缘。
// A tools toggle changes only the bottom band; that frame runs a GL16 differential inside
// the band, with dirty-mask dilation erasing the progress bar edges.
static bool s_reader_tools_toggle;
// 上一次推上屏的面板状态；从有面板变回无面板就是一次收起，要走整屏归一。
// Panel state of the last presented frame; going from a panel back to none is a close and
// takes the whole-screen normalize pass.
static int s_presented_panel;
static char s_requested_open[BOOK_STORE_PATH_MAX];
static bool s_requested_open_home;
static bool s_requested_fullscreen = true;
static bool s_reader_return_home;
static bool s_requested_manage;
static shelf_entry_t* s_shelf;
static book_shelf_covers_t s_shelf_covers;
_Static_assert(BOOK_ROWS == BOOK_SHELF_COVER_SLOTS, "shelf cover slots");
static size_t s_shelf_capacity;
static int s_count, s_visible_count, s_filter;
static bool s_recent_sort;
// 目录记录有界增长；同一开机周期按完整路径保存两种布局的页码。
// Bounded folder records retain pages by full path for the two layouts during this boot.
#define SHELF_CATEGORY_ROWS BOOK_SHELF_CATEGORY_MENU_ROWS
static book_shelf_categories_t s_shelf_categories;
static book_shelf_categories_t s_legacy_categories;
static book_collection_t s_collection;
static uint32_t s_collection_bound_generation;
static book_shelf_covers_t s_category_covers;
static uint32_t s_category_cover_key;
static bool s_shelf_selecting;
static char s_category_result[128], s_cache_library[40];
static bool shelf_collection_load(bool retry);
static bool shelf_collection_sync(bool complete);
static book_shelf_query_t shelf_query(void);
static void category_start_books(app_ctx_t *ctx, uint32_t target, bool review);
static void category_close(app_ctx_t *ctx);
static void category_cover_draw(uint8_t *fb, int row, int index, EpdRect rect);
static void shelf_label(uint8_t *fb, EpdRect rect, int px, const char *text, bool center);
static void shelf_category_init(void);
static void shelf_category_remember(app_ctx_t *ctx);
static void shelf_category_restore(app_ctx_t *ctx);
static void shelf_categories_recount(void);
static void draw_category_menu(uint8_t *fb);
static const char *shelf_category_name(int index);
static int shelf_category_add(const char *path);
static app_redraw_t shelf_category_gesture(app_ctx_t *ctx, const ui_gesture_event_t *ev);
static void category_begin(app_ctx_t *ctx, bool picker, bool bulk);
static void category_draw(uint8_t *fb);
static app_redraw_t category_gesture(app_ctx_t *ctx, const ui_gesture_event_t *ev);
static app_redraw_t category_tick(app_ctx_t *ctx);
static app_redraw_t category_back(app_ctx_t *ctx);
static void category_reset(void);
static bool category_storage_ready(void);
static esp_err_t category_extra_state(const char *old_path, const char *new_path, uint32_t size);
static book_category_ui_t s_category_ui;
static book_view_t s_category_parent;
static bool s_category_bulk, s_category_quick;
static shelf_entry_t s_managed;
static bool s_delete_confirm, s_file_removed;
static char s_shelf_warning[128], s_manage_message[128];
static char s_query[65], s_search_draft[65], s_batch_message[128];
static ui_text_edit_t s_search_input;
static book_view_t s_search_parent;
// 批量操作需要二次确认的三类：删文件、清进度、从书架移除。前两类动数据，后一类只动书架。
// Three batch actions need confirmation: delete files, clear progress, remove from the shelf.
// The first two touch stored data; the last one only touches shelf membership.
typedef enum {
    BATCH_DELETE,
    BATCH_CLEAR,
    BATCH_UNSHELF,
} batch_kind_t;
static bool s_batch_confirm;
static batch_kind_t s_batch_kind;
typedef struct pending_progress {
    char path[BOOK_STORE_PATH_MAX];
    book_progress_t value;
    bool dirty, progress_saved;
    book_progress_watch_t* watch;
    struct pending_progress* next;
} pending_progress_t;
static pending_progress_t* s_pending;
typedef book_shelf_retry_t delete_retry_t;
static delete_retry_t* s_delete_retries;
static char s_latest_path[BOOK_STORE_PATH_MAX];
static char s_editor_title[121], s_editor_notice[96];
static ui_text_edit_t s_editor_input;
static uint8_t* s_editor_cover;
static bool s_save_failed;
static bool s_pending_invalidated;
static int64_t s_save_retry_ms;
static unsigned s_store_revision;
static uint32_t s_progress_revision=UINT32_MAX;
static bool s_shelf_cache_valid, s_cache_sd_present, s_cache_sd_mounted;
static uint8_t s_cache_shelf_style;
static bool s_scan_pending, s_clear_confirm;
static reader_panel_t s_reader_panel;
// 微调长按只更新预览，松手后统一重排和保存；离开控件即取消。
// Held indent adjustments update the preview only; release reflows/saves once and leaving the control cancels.
static struct {
    bool active, cancelled, repeated;
    int control, value;
    int64_t next_ms, last_ms;
} s_indent_hold;
static bool s_indent_feedback_pending;
static void reader_indent_hold_cancel(void);
static bool s_reader_fullscreen;
static bool s_shelf_key_mode,s_shelf_key_paint_pending;
static int s_shelf_key_pressed,s_shelf_key_delta;
static int64_t s_shelf_key_until;
static int64_t s_reader_clock_minute = -1;
static int s_reader_power = -1;
// 已推屏页眉的合成文本指纹；内容不变时不随翻页重复推顶部带。
// Fingerprint of the presented header's composed texts; unchanged content never re-presents the top band on turns.
static uint32_t s_reader_chrome_fp;
static bool s_reader_chrome_fp_valid;
static char s_message[128], s_storage[128];
static char s_chapter_heading_title[READER_HEADING_TITLE_BYTES], s_chapter_heading_label[READER_HEADING_LABEL_BYTES];
static size_t s_chapter_lead_skip;
static unsigned s_chapter_lead_height;
// 横排题头该占的行高，与当前模式无关地记着：首块是插图时它是 0，
// 切回横排要用它，而不是拿竖排留下的 0 去算。
// The row height a horizontal heading should take, remembered independently of the current mode:
// it is zero when the chapter opens with an illustration, and switching back must use it rather
// than the zero a vertical heading left behind.
static unsigned s_chapter_lead_row_height;
// 竖排首页为题头预留的列宽；横排为 0。/ Columns reserved for a vertical heading; 0 horizontally.
static unsigned s_chapter_lead_width;
static char s_font_path[192];
static reader_session_t s_reader_session;
// 迁移期间保留页面的借用视图；已接收章节只由会话释放，交接后统一重绑定。
// Keep borrowed page views during migration; only the session frees adopted chapters and handoffs rebind together.
static char* s_text;
static bool reader_text_body(void);
static bool normalize_hold_gray(app_ctx_t* ctx);
bool app_book_reader_body_visible(void) {
    return s_view == READING && s_text && s_reader_panel == READER_PANEL_NONE && !s_clear_confirm;
}
static blk_t* s_blocks;
// 书内字体的 run 表与块表同生共死：都来自同一次 html_text 解析，排版直接借用。
// The face run table lives and dies with the block table: both come from the same html_text
// parse, and the layout borrows them.
// 内链与锚点和 run 表同源，随章节一起换。点按脚注只查这两张表，不再解析章节。
// Body links and anchors come from the same parse as the run table and change with the
// chapter. A footnote tap only reads these tables; it never re-parses the chapter.
static html_link_t* s_links;
static size_t s_link_count;
static html_anchor_t* s_anchors;
static size_t s_anchor_count;
static char* s_link_strings;
// 注释弹层只保存一段纯文本；脚注通常一两行，2 KB 足够，超长截断。
// The note sheet holds one plain paragraph; footnotes are a line or two, so 2 KB is plenty
// and longer notes are truncated.
#define READER_NOTE_BYTES 2048u
static char s_note_text[READER_NOTE_BYTES];
// 上一次排版用的书内字体开关：字体页改过之后，回阅读页要重排当前章才看得出来。
// The embedded-face setting the current layout was built with: after the font page changes
// it, the reader must re-typeset the chapter before the change is visible.
static bool s_book_fonts_applied;
// 字体开关改了之后要重排当前章，但那次重排不能在触摸回调里做：栈太深。
// The font switch needs the current chapter re-typeset, but not from the touch callback: the
// stack there is too deep.
static bool s_fonts_reload_pending;
static char** s_images;
static size_t s_image_count;
// PR #7 的当前页图片集合：只保留显示页的灰阶，不限制为八张。
// PR #7's current-page image set retains visible grayscale only, without an eight-image limit.
typedef struct { uint8_t *gray; int width, height; } reader_image_t;
static reader_image_t *s_page_images;
static int s_page_image_count;
static size_t s_page_images_for = SIZE_MAX;
static uint32_t s_page_images_generation;
static size_t s_block_count;
static size_t s_text_len;
static size_t s_selected_toc = SIZE_MAX;
static bool s_toc_jump_open, s_toc_jump_drag;
static int s_toc_jump_percent;
static int64_t s_toc_jump_push_ms; ///< 上次推屏时刻，拖动节流 / Last push, throttles drag repaints
static size_t s_jump_offset = SIZE_MAX, s_jump_page;
static int s_px, s_margin[MARGIN_COUNT], s_line_spacing, s_turns, s_unsaved;
static int64_t s_poll_ms, s_last_turn_ms;
static int64_t s_stats_last_ms, s_stats_activity_ms;
// 晃动翻页状态：判定门槛、触摸静默时间与传感器供电标志，采样和生命周期钩子共同维护。
// Shake-turn state: the gate, touch-quiet timestamp and sensor-power flag, maintained by
// sampling and the lifecycle hooks together.
static book_shake_gate_t s_shake_gate;
static int64_t s_shake_touch_ms;
static bool s_shake_sensor_on, s_shake_cfg_pending = true;
static uint32_t s_stats_pending_ms, s_stats_pending_turns;
static uint32_t s_session_read_ms, s_session_turns;
static int s_font_page;
static int s_reader_slider = -1;
static bool s_reader_slider_endpoint;
static int s_reader_preview_px, s_reader_preview_margin[MARGIN_COUNT], s_reader_preview_line, s_reader_preview_para;
static int s_reader_preview_tracking;
static uint32_t s_recent_days[30];
static bool s_recent_days_valid;
static char s_reader_notice[64];
static int64_t s_reader_notice_until;
static int s_bookmark_page;
static bool s_bookmark_edit, s_bookmark_delete_confirm, s_bookmark_delete_error;
static uint32_t s_bookmark_selected;
static EpdRect s_area;
static enum EpdDrawMode s_mode = MODE_GL16;
static bool s_input_settle;
static bool s_reader_cleanup;
// 开书首帧：上一页的深灰边线和文字靠差分擦不干净，首帧改走整屏 GC16。
// Opening frame: a differential cannot fully erase the previous page's dark frame lines and
// text, so the first frame takes a whole-screen GC16 instead.
static bool s_reader_entry_full;
static bool s_reader_footer_pending;
// 换章这一次页脚改用 37 相 GL16 差分，其余翻页仍是 8 相 DU。
// A chapter-change turn presents the footer with the 37-phase GL16 differential; other turns keep the 8-phase DU.
static bool s_reader_footer_strong;
static bool s_water_turn_pending;
static e0470_turn_dir_t s_water_turn_dir;
static int s_pressed_control = -1;
static bool s_shelf_feedback_pending;
static bool s_cover_batch_present_pending;
static bool s_shelf_page_pending;
static unsigned s_reader_key_pick;
static char s_reader_key_error[64];
// 页眉页脚设置面板状态：选中槽位、列表滚动与保存错误提示。
// Chrome settings panel state: the picked slot, list scroll and the save-error notice.
static unsigned s_chrome_pick_bar;   // 0=页眉 1=页脚 / 0 header, 1 footer
static unsigned s_chrome_pick_slot;  // 0/1/2 = 左/中/右 / left/center/right
static int s_chrome_scroll;
static int s_chrome_height[2] = {APP_READER_HEADER_HEIGHT_DEFAULT, APP_READER_FOOTER_HEIGHT_DEFAULT};
static bool s_chrome_drag;
static int s_chrome_pick_scroll;
static bool s_chrome_pick_drag;
static char s_chrome_error[64];
// 中键双击：绑定动作后，单击先挂起一个双击窗口，超时且无触摸才补发。
// Middle double press: once an action is bound, a single click is held for the double-press
// window and replayed only after it expires with no contact.
#define READER_MIDDLE_DOUBLE_MS 340
static bool s_middle_pending;
static app_reader_key_action_t s_middle_pending_action;
static int64_t s_middle_pending_ms;
static void middle_pending_clear(void) { s_middle_pending = false; }
static int64_t s_du_ms;
static unsigned s_du_count;
static EpdRect s_du_area;

#define s_toolbar (s_reader_panel != READER_PANEL_NONE)

static void render(app_ctx_t* ctx, uint8_t* fb);
static void draw_reader_panel(uint8_t* fb);
static EpdRect reader_slider_rect(int slider);
static const char* reader_title_style_name(int style);
static void scan_shelf(app_ctx_t* ctx);
static void copy_text(char* dst, size_t cap, const char* src);
static void fit_text(char* text, int px, int width);
static void free_book(void);
static void save_progress(void);
static void invalidate_prep(void);
static void prepare_inline_image(void);
static void release_page_images(void);
static void sort_shelf(app_ctx_t* ctx);
static void prepare_covers(app_ctx_t* ctx);
static app_redraw_t toggle_reader_fullscreen(app_ctx_t* ctx);
bool app_book_request_open(const char* path) {
    if (!path || !path[0] || strnlen(path, sizeof(s_requested_open)) >= sizeof(s_requested_open)) return false;
    if (!book_file_path_supported(path)) return false;
    copy_text(s_requested_open, sizeof(s_requested_open), path);
    s_requested_open_home = false;
    // 从书架进书沿用用户记忆的全屏偏好（默认全屏）；锁屏恢复链路仍用显式参数。
    // Shelf entries follow the remembered full-screen preference (default on); the sleep
    // resume path keeps its explicit argument.
    s_requested_fullscreen = app_settings_reader_open_fullscreen();
    return true;
}
bool app_book_request_open_from_home(const char* path) {
    if (!app_book_request_open(path)) return false;
    s_requested_open_home = true;
    return true;
}
bool app_book_request_resume(const char *path, bool fullscreen) {
    if (!app_book_request_open_from_home(path)) return false;
    s_requested_fullscreen = fullscreen;
    return true;
}
bool app_book_request_return_to_body(const char *path, bool fullscreen) {
    // 走普通开书来源：正文的返回仍是书架，不继承深睡恢复的「回首页」语义。
    // Use the ordinary open origin: the body still returns to the shelf, unlike the
    // deep-sleep resume's "back to home" semantics.
    if (!app_book_request_open(path)) return false;
    s_requested_fullscreen = fullscreen;
    return true;
}
bool app_book_resume_context(char *path, size_t capacity, bool *fullscreen) {
    if (!s_text || !s_reader_session.book.path[0] || s_view == SHELF || s_view == MANAGE || s_view == BULK ||
        !path || !fullscreen || strlen(s_reader_session.book.path) >= capacity || s_save_failed) return false;
    copy_text(path, capacity, s_reader_session.book.path);
    *fullscreen = s_reader_fullscreen;
    return true;
}
void app_book_request_manage(void) { s_requested_manage = true; }
static void draw_control(uint8_t* fb, EpdRect rect, const char* label, int id) {
    if (id == 112 && s_query[0]) {
        ui_fill_round_rect(fb, rect, UI_BTN_RADIUS, UI_GRAY_BLACK);
        ui_text_vc(fb, rect.x + rect.width / 2, rect.y + rect.height / 2,
                   UI_PX_BTN, label, EPD_DRAW_ALIGN_CENTER, true);
        return;
    }
    if (s_pressed_control == id) ui_draw_pressed_round_rect(fb, rect, UI_BTN_RADIUS);
    ui_draw_button(fb, rect, label, false);
}
static void lock_draw(void) {
    reader_prefetch_lock();
}
static void unlock_draw(void) { reader_prefetch_unlock(); }
static size_t fb_bytes(void) { return (size_t)epd_width() * epd_height() / 2; }
#define READER_FULLSCREEN_PROGRESS_TOP (UI_LOCK_HEIGHT - 12)
// 正文保留顶部呼吸空间，下沿与可调页脚带相接；全屏沿用原有几何。
// Keep the top breathing space and meet the adjustable footer band; full screen retains its geometry.
static EpdRect reader_area(void) {
    const int top = s_reader_fullscreen
        ? (app_settings_reader_statusbar() ? 80 : 0)
        : (app_settings_reader_statusbar() ? 80 : 0) + s_chrome_height[0];
    // 全屏进度线单独差分，翻页时不让正文刷新反复驱动整条横线。
    // Keep the full-screen progress strip out of body updates so turns do not drive the whole bar.
    const int bottom = s_reader_fullscreen ? READER_FULLSCREEN_PROGRESS_TOP : UI_BAR_TOP + UI_BAR_H - s_chrome_height[1];
    return (EpdRect){0, top, UI_LOCK_WIDTH, bottom - top};
}
static EpdRect reader_fullscreen_progress_area(void) {
    return (EpdRect){0, READER_FULLSCREEN_PROGRESS_TOP,
                     UI_LOCK_WIDTH, UI_LOCK_HEIGHT - READER_FULLSCREEN_PROGRESS_TOP};
}
// 工具栏覆盖底部 120px；页脚进度条（1143–1155）和章节行都在这条带内。
// The tools bar covers the bottom 120 px; the footer bar (1143–1155) and title row sit inside it.
static EpdRect reader_tools_area(void) {
    return (EpdRect){0, UI_BAR_TOP, UI_LOCK_WIDTH, UI_LOCK_HEIGHT - UI_BAR_TOP};
}
static EpdRect body_rect_for_tracking(int tracking_index) {
    (void)tracking_index;
    int top = s_reader_fullscreen
        ? (app_settings_reader_statusbar() ? 96 : 24)
        : (app_settings_reader_statusbar() ? 96 : 16) + s_chrome_height[0];
    // 正文底与页脚差分带相接。/ Body bottom meets the footer refresh band.
    int bottom = s_reader_fullscreen ? READER_FULLSCREEN_PROGRESS_TOP - 2 : UI_BAR_TOP + UI_BAR_H - s_chrome_height[1];
    // 四向留白是屏幕边距，横排竖排同一套：左右缩栏宽，上下缩页高。
    // 不吸附整字格，完整行把余量摊到字间。
    // The four insets are screen-side margins shared by both writing modes: left/right narrow the
    // column, top/bottom shorten the page. They never snap to glyph cells; full lines spread the
    // remainder into the tracking.
    return (EpdRect){s_margin[MARGIN_LEFT], top + s_margin[MARGIN_TOP],
                     UI_LOCK_WIDTH - s_margin[MARGIN_LEFT] - s_margin[MARGIN_RIGHT],
                     bottom - top - s_margin[MARGIN_TOP] - s_margin[MARGIN_BOTTOM]};
}
static EpdRect body_rect(void) {
    return body_rect_for_tracking(app_settings_book_tracking());
}
// 边距档位是固定九档，不再按字体量宽去重：四向共用同一条阶梯。
// Margin stops are a fixed nine-step ladder shared by all four sides; no width deduplication.
static int reader_margin_level(int px) {
    int level = (px + BOOK_MARGIN_STEP / 2) / BOOK_MARGIN_STEP;
    if (level < 0) level = 0;
    if (level >= BOOK_MARGIN_CHOICES) level = BOOK_MARGIN_CHOICES - 1;
    return level;
}
static int reader_margin_value(int level) {
    if (level < 0) level = 0;
    if (level >= BOOK_MARGIN_CHOICES) level = BOOK_MARGIN_CHOICES - 1;
    return BOOK_MARGIN_MIN + level * BOOK_MARGIN_STEP;
}
static EpdRect progress_rect(void) {
    // 固定底边与左右边距，增高时向正文方向扩展；进度条和文字随上沿移动。
    // Keep the bottom and side insets fixed; growth expands toward the body, moving the bar and text with its top.
    return (EpdRect){36, UI_BAR_TOP + UI_BAR_H - s_chrome_height[1], UI_LOCK_WIDTH - 72, s_chrome_height[1]};
}
static int shelf_rows(void) { return s_view == SHELF && app_settings_shelf_style() == 5 ? 4 : BOOK_GRID_ROWS; }
#define SHELF_BOOK_LIFT_PX 16
#define SHELF_LIST_BOUNCE_PX 8
#define SHELF_FAST_COVER_WIDTH 164
#define SHELF_FAST_COVER_HEIGHT 214
#define SHELF_FAST_COVER_STRIDE ((SHELF_FAST_COVER_WIDTH + 7) / 8)
#define SHELF_FAST_COVER_BYTES (SHELF_FAST_COVER_STRIDE * SHELF_FAST_COVER_HEIGHT)
#define SHELF_FAST_CACHE_RESERVE (512u * 1024u)
#define SHELF_GUARD_CACHE_BYTES (644u * 90u / 2u)
typedef struct { EpdRect rect; int rotation, contrast; bool binary, valid; } shelf_guard_key_t;
static uint8_t *s_guard_cache;
static shelf_guard_key_t s_guard_keys[3];
static bool s_guard_cache_attempted;
static void release_guard_cache(void) {
    free(s_guard_cache); s_guard_cache=NULL; s_guard_cache_attempted=false;
    memset(s_guard_keys,0,sizeof(s_guard_keys));
}
// 缓存精确的输入和成品；逐行比对底图，任何封面、位置或模式变化均重绘。
// Cache exact inputs and finished pixels; compare every input row so art, position and mode changes redraw.
static void draw_shelf_guard(uint8_t *fb,EpdRect rect,int slot) {
    const bool binary=s_view==SHELF&&app_settings_main_fast_refresh();
    EpdRect p=ui_rotate_rect_to_fb(rect);
    const bool fits=slot>=0&&slot<3&&!(p.x&1)&&!(p.width&1)&&p.x>=0&&p.y>=0&&
        p.x+p.width<=epd_width()&&p.y+p.height<=epd_height()&&
        (size_t)p.width*p.height/2<=SHELF_GUARD_CACHE_BYTES;
    if(fits&&!s_guard_cache&&!s_guard_cache_attempted) {
        s_guard_cache_attempted=true;
        size_t bytes=6u*SHELF_GUARD_CACHE_BYTES;
        if(heap_caps_get_free_size(MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT)>=bytes+2u*1024u*1024u)
            s_guard_cache=heap_caps_malloc(bytes,MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT);
    }
    if(!fits||!s_guard_cache){ui_draw_acrylic_guard(fb,rect);if(binary)ui_acrylic_bw_rect(fb,rect);return;}
    shelf_guard_key_t *key=&s_guard_keys[slot];
    size_t stride=(size_t)epd_width()/2,span=(size_t)p.width/2;
    uint8_t *input=s_guard_cache+2u*slot*SHELF_GUARD_CACHE_BYTES,*output=input+SHELF_GUARD_CACHE_BYTES;
    bool same=key->valid&&key->rotation==epd_get_rotation()&&key->contrast==app_settings_system_contrast()&&
        key->binary==binary&&!memcmp(&key->rect,&rect,sizeof(rect));
    for(int y=0;same&&y<p.height;++y)
        same=!memcmp(fb+(size_t)(p.y+y)*stride+p.x/2,input+(size_t)y*span,span);
    if(!same) {
        key->valid=false;
        for(int y=0;y<p.height;++y)memcpy(input+(size_t)y*span,fb+(size_t)(p.y+y)*stride+p.x/2,span);
        ui_draw_acrylic_guard(fb,rect);if(binary)ui_acrylic_bw_rect(fb,rect);
        for(int y=0;y<p.height;++y)memcpy(output+(size_t)y*span,fb+(size_t)(p.y+y)*stride+p.x/2,span);
        *key=(shelf_guard_key_t){.rect=rect,.rotation=epd_get_rotation(),.contrast=app_settings_system_contrast(),.binary=binary,.valid=true};
    } else for(int y=0;y<p.height;++y)memcpy(fb+(size_t)(p.y+y)*stride+p.x/2,output+(size_t)y*span,span);
}
static EpdRect row_rect(int row) {
    if (s_view == SHELF && app_settings_shelf_style() == 5)
        return s_shelf_selecting?(EpdRect){36,280+row*180,612,180}:(EpdRect){36,184+row*197,612,197};
    if (s_view != BULK) {
        int col = row % 3, line = row / 3;
        int cell = (ui_content_width() - 2 * UI_GAP) / 3;
        if(s_view==SHELF && s_shelf_selecting)
            return (EpdRect){UI_MARGIN+col*(cell+UI_GAP),280+line*244,cell,240};
        return (EpdRect){UI_MARGIN + col * (cell + UI_GAP), (s_view == SHELF ? 192 : 224) + line * 282, cell, 260};
    }
    return (EpdRect){UI_MARGIN, 308 + row * (UI_BTN_H + UI_GAP), ui_content_width(), UI_BTN_H};
}
static EpdRect shelf_cover_image(int row) {
    EpdRect card = row_rect(row);
    if (s_view != SHELF || app_settings_shelf_style() != 5)
        return (EpdRect){card.x + (card.width - 164) / 2, card.y, 164, 214};
    // 列表所有封面统一尺寸；原图按比例铺满并居中裁切。/ Uniform list boxes use aspect fill and center crop.
    return (EpdRect){71, card.y + (197-156)/2, 120, 156};
}
static void invalidate_covers(void) {
    book_shelf_covers_cancel(&s_shelf_covers);
    release_guard_cache();
    book_shelf_covers_release(&s_shelf_covers);
}
static void release_fast_covers(void) {
    for (int row = 0; row < BOOK_ROWS; ++row) book_shelf_covers_drop_fast(&s_shelf_covers, row);
}
static bool fast_cover_matches(int row, EpdRect image) {
    return book_shelf_covers_fast_matches(&s_shelf_covers, row, app_settings_system_contrast(),
                                          (image.x & 3) | ((image.y & 3) << 2));
}
static bool cover_favorite_needs_white_edge(const uint8_t *gray, EpdRect image, unsigned sw, unsigned sh);
static void prepare_fast_covers(void) {
    // 转换成功即释放灰阶；不释放仍有效的黑白缓存，避免重复切页重新解码。
    // Release gray immediately after conversion, retaining valid packed artwork across tab changes.
    if (s_view != SHELF || !app_settings_main_fast_refresh()) { release_fast_covers(); return; }
    for (int row = 0; row < BOOK_ROWS; ++row) {
        const book_shelf_cover_t *cover = book_shelf_covers_slot(&s_shelf_covers, row);
        if (!cover->gray) continue;
        EpdRect image = shelf_cover_image(row);
        if (row < shelf_rows() && row < BOOK_GRID_ROWS && !fast_cover_matches(row, image)) {
            book_shelf_covers_drop_fast(&s_shelf_covers, row);
            // 转换后回收的灰阶空间也计入保留量；内存不足时显示标题，不长期保留灰阶副本。
            // Count reclaimed gray toward the reserve; on OOM show a title rather than retain a gray duplicate.
            size_t free_after = heap_caps_get_free_size(MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT) + BOOK_COVER_W * BOOK_COVER_H;
            uint8_t *bits = free_after >= SHELF_FAST_CACHE_RESERVE + SHELF_FAST_COVER_BYTES
                ? heap_caps_calloc(1, SHELF_FAST_COVER_BYTES, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT) : NULL;
            if (bits) {
                unsigned sw = app_settings_shelf_style() == 5 ? cover->width : BOOK_COVER_W;
                unsigned sh = app_settings_shelf_style() == 5 ? cover->height : BOOK_COVER_H;
                book_crop_t crop = book_cover_crop(sw, sh, image.width, image.height);
                unsigned sample_x[SHELF_FAST_COVER_WIDTH];
                uint8_t tones[256];
                for(int x=0;x<image.width;++x)sample_x[x]=crop.x+(uint64_t)(unsigned)x*crop.width/(unsigned)image.width;
                for(unsigned v=0;v<256;++v)tones[v]=(ui_contrast_gray((uint8_t)v)>>4)*17u;
                for (int y = 0; y < image.height; ++y) {
                    const unsigned sy = crop.y + (uint64_t)(unsigned)y * crop.height / (unsigned)image.height;
                    for (int x = 0; x < image.width; ++x) {
                        const unsigned sx = sample_x[x];
                        uint8_t tone = tones[cover->gray[sy * sw + sx]];
                        if (ui_image_dither_cover_bw(tone, image.x + x, image.y + y))
                            bits[y * SHELF_FAST_COVER_STRIDE + x / 8] |= (uint8_t)(1u << (x & 7));
                    }
                }
                book_shelf_covers_set_fast(&s_shelf_covers, row, bits,
                    cover_favorite_needs_white_edge(cover->gray, image, sw, sh),
                    app_settings_system_contrast(), (image.x & 3) | ((image.y & 3) << 2));
            }
        }
        book_shelf_covers_drop_gray(&s_shelf_covers, row);
    }
}
void app_book_cover_mode_changed(void) { invalidate_covers(); }
static uint8_t* load_cover_gray(const char* source, const char* title, const char* author,
                                bool decode, bool* pending) {
    *pending = false;
    if (!pico_boot_asset_allowed(source)) return NULL;
    uint8_t* gray = heap_caps_malloc(BOOK_COVER_W * BOOK_COVER_H, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (!gray) return NULL;
    if (!book_cover_load_gray(source, title, author, gray, decode, pending)) {
        free(gray); return NULL;
    }
    return gray;
}
// 直接读最终点阵：不分配灰阶工作区，也不在每次书架切页重新转换。
// Read the finished raster: no gray workspace or repeated conversion on shelf paging.
static bool load_shelf_cover_fast(int row, int index) {
    if (s_view != SHELF || !app_settings_main_fast_refresh() || row >= BOOK_GRID_ROWS ||
        !pico_boot_asset_allowed(s_shelf[index].path)) return false;
    EpdRect image = shelf_cover_image(row);
    size_t bytes = (size_t)SHELF_FAST_COVER_STRIDE * image.height;
    if (bytes > SHELF_FAST_COVER_BYTES || heap_caps_get_free_size(MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT)
        < SHELF_FAST_CACHE_RESERVE + SHELF_FAST_COVER_BYTES) return false;
    uint8_t *bits = heap_caps_calloc(1, SHELF_FAST_COVER_BYTES, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (!bits) return false;
    book_fast_cover_key_t key = {(unsigned)image.width, (unsigned)image.height, SHELF_FAST_COVER_STRIDE,
        app_settings_system_contrast(), (image.x&3)|((image.y&3)<<2)};
    bool edge;
    if (!book_cover_fast_cache_read(s_shelf[index].path, s_shelf[index].name, s_shelf[index].author,
                                   key, bits, &edge)) { free(bits); return false; }
    book_shelf_covers_set_fast(&s_shelf_covers, row, bits, edge, key.contrast, key.phase);
    return true;
}
static void save_shelf_cover_fast(int row, int index) {
    const book_shelf_cover_t *cover = book_shelf_covers_slot(&s_shelf_covers, row);
    if (!cover->fast_bits) return;
    EpdRect image = shelf_cover_image(row);
    book_fast_cover_key_t key = {(unsigned)image.width, (unsigned)image.height, SHELF_FAST_COVER_STRIDE,
        cover->fast_contrast, cover->fast_phase};
    book_cover_fast_cache_write(s_shelf[index].path, s_shelf[index].name, s_shelf[index].author,
                                key, cover->fast_bits, cover->fast_white_edge);
}

static unsigned prepare_cover_batch(app_ctx_t *ctx,bool cache_only) {
    if((s_view!=SHELF&&s_view!=MANAGE)||(!cache_only&&ctx->touch&&ctx->touch->touched))return 0;
    unsigned changed=0;
    for(int row=0;row<shelf_rows();++row){
        unsigned bit=1u<<row;if(!(book_shelf_covers_pending(&s_shelf_covers)&bit))continue;
        if(s_view==SHELF&&app_poll_main_navigation(ctx))return changed;
        int index=ctx->leaf*shelf_rows()+row;
        if(index<0||index>=s_visible_count||book_shelf_covers_slot(&s_shelf_covers, row)->index!=index||s_shelf[index].removed){book_shelf_covers_skip(&s_shelf_covers, row);continue;}
        EpdRect image=shelf_cover_image(row);
        book_cover_job_t request={.list=s_view==SHELF&&app_settings_shelf_style()==5,
            .fast=s_view==SHELF&&app_settings_main_fast_refresh(),.width=image.width,.height=image.height,
            .stride=SHELF_FAST_COVER_STRIDE,.contrast=app_settings_system_contrast(),
            .phase=(image.x&3)|((image.y&3)<<2),.x=image.x,.y=image.y};
        copy_text(request.path,sizeof(request.path),s_shelf[index].path);
        copy_text(request.title,sizeof(request.title),s_shelf[index].name);
        copy_text(request.author,sizeof(request.author),s_shelf[index].author);
        if (book_shelf_covers_collect(&s_shelf_covers, row, &request)) changed |= bit;
    }
    return changed;
}

// 分批只用于准备，不能让几张已解码封面先出现在屏幕或点击反馈中。
// Chunk preparation only: neither page painting nor press feedback reveals an incomplete cover group.
static bool shelf_cover_frame_ready(void) {
    return s_view != SHELF || book_shelf_covers_pending(&s_shelf_covers) == 0;
}
static unsigned shelf_cover_reveal_mask(void) {
    return book_shelf_covers_reveal(&s_shelf_covers, s_view == SHELF);
}

static void prepare_covers(app_ctx_t* ctx) {
    if (s_view != SHELF && s_view != MANAGE) return;
    int rows = shelf_rows();
    book_shelf_covers_page(&s_shelf_covers, ctx->leaf * rows);
    for (int row = 0; row < rows; ++row) {
        int index = ctx->leaf * rows + row;
        EpdRect image = shelf_cover_image(row);
        book_shelf_covers_prepare(&s_shelf_covers, row, index,
            index >= 0 && index < s_visible_count && !s_shelf[index].removed,
            s_view == SHELF && app_settings_main_fast_refresh(), app_settings_system_contrast(),
            (image.x & 3) | ((image.y & 3) << 2), ctx->now_ms);
    }
    prepare_fast_covers();
    (void)prepare_cover_batch(ctx, true);
}

// 首帧短暂汇合TF缓存命中，让题头、书架和封面一起显示；冷解码不无限阻塞。
// Briefly gather warm TF hits for a complete first frame; never wait indefinitely for cold decoding.
static void gather_cover_entry(app_ctx_t *ctx) {
    if(s_view!=SHELF||!book_shelf_covers_pending(&s_shelf_covers)||ctx->request_app||
       (s_presented_view==(int)SHELF&&!s_shelf_page_pending))return;
    const int64_t deadline=esp_timer_get_time()+60000;
    while(book_shelf_covers_pending(&s_shelf_covers)&&esp_timer_get_time()<deadline){
        if(app_poll_main_navigation(ctx))return;
        vTaskDelay(1);
        (void)prepare_cover_batch(ctx,true);
        if(ctx->request_app)return;
    }
}

// 画面和操作优先，预生成只在封面加载完、无触摸的空闲书架运行一次一本。
// Present and input take priority; warm one cover only on an idle shelf after visible loading completes.
static void warm_shelf_lock_cover(app_ctx_t *ctx) {
    if (s_view != SHELF || app_settings_lock_style() != 2 || s_clear_confirm || s_batch_confirm ||
        !book_shelf_covers_warm_due(&s_shelf_covers, ctx->now_ms) ||
        (ctx->touch && ctx->touch->touched) || ctx->request_app || ctx->request_return || ctx->request_menu) return;
    if (app_poll_main_navigation(ctx)) return;
    int row = book_shelf_covers_warm_take(&s_shelf_covers, shelf_rows(), ctx->now_ms);
    if (row < 0) return;
    int index = ctx->leaf * shelf_rows() + row;
    if (index >= 0 && index < s_visible_count && book_shelf_covers_slot(&s_shelf_covers, row)->index == index && !s_shelf[index].removed)
        (void)book_lock_collage_warm_cover(s_shelf[index].path);
    (void)app_poll_main_navigation(ctx);
}

static void draw_favorite_icon(uint8_t *fb, int x, int y, int w, int h, bool filled, uint8_t ink) {
    // 只绘制标记本身，封面上不加矩形衬底。/ Draw only the glyph, without a rectangular cover underlay.
    const int notch_y = y + h - 9;
    if (filled) {
        // 把 V 形尾部也填满，避免选中后只剩矩形黑块、尾部仍是空心。
        // Fill the V-shaped tail as well so the selected glyph is solid throughout.
        for (int py = y; py <= y + h; ++py) {
            int arm = py <= notch_y ? w / 2 : (w / 2) * (y + h - py) / 9;
            ui_fill_rect_fast((EpdRect){x, py, arm + 1, 1}, ink, fb);
            ui_fill_rect_fast((EpdRect){x + w - arm, py, arm + 1, 1}, ink, fb);
        }
        return;
    }
    // 空心态使用与阅读工具栏相近的两像素线宽。
    // Keep the outline close to the other reader toolbar icons in visual weight.
    ui_fill_rect_fast((EpdRect){x, y, w + 1, 2}, ink, fb);
    ui_fill_rect_fast((EpdRect){x, y, 2, h - 1}, ink, fb);
    ui_fill_rect_fast((EpdRect){x + w - 1, y, 2, h - 1}, ink, fb);
    epd_draw_line(x, y + h, x + w / 2, notch_y, ink, fb);
    epd_draw_line(x + 1, y + h - 1, x + w / 2, notch_y + 1, ink, fb);
    epd_draw_line(x + w / 2, notch_y, x + w, y + h, ink, fb);
    epd_draw_line(x + w / 2, notch_y + 1, x + w - 1, y + h - 1, ink, fb);
}
static void draw_shelf_favorite_icon(uint8_t *fb, int x, int y, int w, int h, bool white_edge) {
    if (white_edge) {
        // 仅沿书签轮廓扩两像素白边，深封面上也不出现白色矩形底块。
        // Dilate only the bookmark silhouette by two pixels, never a white backing box.
        const int notch_y = y + h - 9;
        for (int py = y; py <= y + h; ++py) {
            int arm = py <= notch_y ? w / 2 : (w / 2) * (y + h - py) / 9;
            ui_fill_rect_fast((EpdRect){x - 2, py - 2, arm + 5, 5}, UI_GRAY_WHITE, fb);
            ui_fill_rect_fast((EpdRect){x + w - arm - 2, py - 2, arm + 5, 5}, UI_GRAY_WHITE, fb);
        }
    }
    draw_favorite_icon(fb, x, y, w, h, true, UI_GRAY_BLACK);
}
static bool cover_favorite_needs_white_edge(const uint8_t *gray, EpdRect image, unsigned sw, unsigned sh) {
    if (!gray || !sw || !sh || image.width <= 0 || image.height <= 0) return false;
    unsigned sum = 0, count = 0;
    const book_crop_t crop = book_cover_crop(sw, sh, image.width, image.height);
    // 只在标记周围采样；图标始终为黑色，深封面才加白边。
    // Sample the badge area; keep the glyph black and outline it only on dark covers.
    for (int y = 9; y < 39; y += 5) {
        for (int x = 11; x < 35; x += 4) {
            unsigned sx = crop.x + (uint64_t)(unsigned)x * crop.width / (unsigned)image.width;
            unsigned sy = crop.y + (uint64_t)(unsigned)y * crop.height / (unsigned)image.height;
            if (sx >= sw) sx = sw - 1;
            if (sy >= sh) sy = sh - 1;
            sum += ui_contrast_gray(gray[sy * sw + sx]);
            ++count;
        }
    }
    return sum < count * 145;
}
static void draw_shelf_cover(uint8_t* fb, EpdRect card, int row, const char* name, bool favorite) {
    const book_shelf_cover_t *cover = book_shelf_covers_slot(&s_shelf_covers, row);
    EpdRect image = shelf_cover_image(row);
    image.y += card.y - row_rect(row).y;
    const bool artwork = shelf_cover_frame_ready();
    ui_fill_rect_fast(image, UI_GRAY_LIGHT, fb);
    if (artwork && app_settings_main_fast_refresh() && s_view==SHELF && fast_cover_matches(row,image)) {
        ui_blit_mono_bitmap(fb,image,cover->fast_bits,SHELF_FAST_COVER_STRIDE);
    } else if (artwork && cover->gray) {

        const uint8_t *gray = cover->gray;
        // 封面缓冲是 176×240、这一格是 164×214；按长边铺满 + 居中裁剪，避免被压扁。
        // The cover buffer is 176x240 and this frame is 164x214; fill by the longer side and
        // centre-crop so the artwork is not squashed.
        const unsigned frame_width = (unsigned)image.width, frame_height = (unsigned)image.height;
        unsigned sw = s_view == SHELF && app_settings_shelf_style() == 5 ? cover->width : BOOK_COVER_W;
        unsigned sh = s_view == SHELF && app_settings_shelf_style() == 5 ? cover->height : BOOK_COVER_H;
        const book_crop_t crop = book_cover_crop(sw, sh, frame_width, frame_height);
        unsigned sample_x[SHELF_FAST_COVER_WIDTH];
        uint8_t tones[256];
        const bool sampled=image.width<=SHELF_FAST_COVER_WIDTH;
        const bool binary=app_settings_main_fast_refresh()&&s_view==SHELF;
        if(sampled)for(int x=0;x<image.width;++x)sample_x[x]=crop.x+(uint64_t)(unsigned)x*crop.width/frame_width;
        for(unsigned v=0;v<256;++v)tones[v]=ui_contrast_gray((uint8_t)v);
        for (int y = 0; y < image.height; ++y) {
            const int sy = (int)(crop.y + (uint64_t)(unsigned)y * crop.height / frame_height);
            for (int x = 0; x < image.width; ++x) {
                const unsigned sx = sampled?sample_x[x]:crop.x+(uint64_t)(unsigned)x*crop.width/frame_width;
                const uint8_t tone = tones[gray[sy * sw + sx]];
                epd_draw_pixel(image.x + x, image.y + y,
                               binary
                                   ? ui_image_dither_cover_bw((tone>>4)*17u, image.x + x, image.y + y)
                                   : ui_image_dither_gray(tone, image.x + x, image.y + y), fb);
            }
        }
    } else {
        char title[80]; copy_text(title, sizeof(title), name);
        char *dot = strrchr(title, '.'); if (dot) *dot = 0;
        fit_text(title, 26, image.width - 22);
        ui_text_vc(fb, image.x + image.width / 2, image.y + image.height / 2, 26,
                   title, EPD_DRAW_ALIGN_CENTER, false);
    }
    ui_draw_round_rect(fb, image, 0, UI_GRAY_BLACK);
    if (favorite && image.width >= 28 && image.height >= 42) {
        draw_shelf_favorite_icon(fb, image.x + 10, image.y + 8, 24, 31,
                                 artwork && ((app_settings_main_fast_refresh() && s_view == SHELF && fast_cover_matches(row, image))
                                     ? cover->fast_white_edge : cover_favorite_needs_white_edge(cover->gray, image,
                                         app_settings_shelf_style() == 5 ? cover->width : BOOK_COVER_W,
                                         app_settings_shelf_style() == 5 ? cover->height : BOOK_COVER_H)));
    }
}
static EpdRect paint_list_cover_feedback(app_ctx_t *ctx, int row) {
    EpdRect image = shelf_cover_image(row);
    EpdRect area = image;
    area.y -= SHELF_LIST_BOUNCE_PX;
    area.height += SHELF_LIST_BOUNCE_PX;
    // 两位置均位于白色列表卡片内；不清整行，也不绘制额外反馈框。
    // Both positions stay within the white list card; never clear a whole row or add a feedback frame.
    ui_fill_rect_fast(area, UI_GRAY_WHITE, ctx->fb);
    EpdRect card = row_rect(row);
    if (s_pressed_control == row) card.y -= SHELF_LIST_BOUNCE_PX;
    int index = ctx->leaf * shelf_rows() + row;
    if (index >= 0 && index < s_visible_count)
        draw_shelf_cover(ctx->fb, card, row, s_shelf[index].name, s_shelf[index].favorite);
    return area;
}
static void draw_list_title(uint8_t *fb, const char *name, int top) {
    char lines[2][128] = {{0}};
    int px = ttf_em_height_px(27);
    const unsigned char *at = (const unsigned char *)name;
    for (int line = 0; line < 2 && *at; ++line) {
        size_t used = 0;
        while (*at && used < sizeof(lines[line])-5) {
            unsigned n = *at < 128 ? 1 : (*at & 0xe0)==0xc0 ? 2 : (*at & 0xf0)==0xe0 ? 3 : 4;
            if (strlen((const char *)at) < n) break;
            memcpy(lines[line]+used, at, n); lines[line][used+n] = 0;
            if (ui_text_fixed_width_px(px, lines[line]) > 378) { lines[line][used] = 0; break; }
            used += n; at += n;
        }
        if (line == 1 && *at) fit_text(lines[line], px, 352);
        ui_text_fixed_vc(fb, 244, top + line*35, px, lines[line], EPD_DRAW_ALIGN_LEFT, false);
    }
}
static void draw_list_row(uint8_t *fb, int row, const shelf_entry_t *item) {
    EpdRect card = row_rect(row);
    draw_list_title(fb, item->name, card.y + 42);
    char author[128]; copy_text(author, sizeof(author), item->author[0] ? item->author : "作者未知");
    int author_px = ttf_em_height_px(21), progress_px = ttf_em_height_px(19);
    fit_text(author, author_px, 378);
    int author_y = ui_text_fixed_width_px(ttf_em_height_px(27), item->name) > 378 ? 106 : 83;
    ui_text_fixed_ink_vc(fb, 244, card.y + author_y, author_px, author, EPD_DRAW_ALIGN_LEFT, 0x60);
    unsigned progress = item->has_progress ? item->pct : 0;
    if (progress > 100) progress = 100;
    char label[48]; snprintf(label, sizeof(label), progress == 100 ? "已读完 · %u%%" :
                              progress ? "已读 %u%%" : "未开始 · %u%%", progress);
    ui_text_fixed_vc(fb, 244, card.y + 143, progress_px, label, EPD_DRAW_ALIGN_LEFT, false);
    EpdRect bar = {244, card.y + 163, 378, 11};
    ui_fill_round_rect(fb, bar, 5, 0xd0);
    if (progress) {
        EpdRect fill = bar; fill.width = bar.width * progress / 100;
        if (fill.width < 1) fill.width = 1;
        ui_fill_round_rect(fb, fill, 5, 0x40);
    }
    ui_draw_control_frame(fb, bar, 5, 0x50);
    if (row < 3) ui_draw_separator(fb, card.y + card.height-1, 50, 584, 0x60);
}
static void draw_shelf_furniture(uint8_t* fb) {
    uint8_t style = app_settings_shelf_style();
    if (!style || style == 5) return;
    for (int row = 0; row < 3; ++row) {
        int top = row_rect(row*3).y;
        if (style == 1) {
            ui_fill_rect_fast((EpdRect){36, top + 214, 612, 12}, UI_GRAY_BLACK, fb);
            ui_hairline(fb, top + 214, 36, 612, 0x58);
            ui_hairline(fb, top + 225, 36, 612, 0x10);
        } else if (style == 2) {
            EpdRect acrylic = {20, top + 181, 644, 45};
            draw_shelf_guard(fb, acrylic, row);

        }
    }
}
static EpdRect tool_rect(int i) {
    int x0 = i * UI_LOCK_WIDTH / BOOK_TOOL_COUNT;
    int x1 = (i + 1) * UI_LOCK_WIDTH / BOOK_TOOL_COUNT;
    return (EpdRect){x0, 1096, x1 - x0, 120};
}
static int leaves(void) {
    if (s_view == TOC) return book_toc_pages(book_navigation_count());
    int count = s_visible_count;
    int rows = s_view == BULK ? BOOK_BULK_ROWS : shelf_rows();
    return count ? 1 + (count - 1) / rows : 1;
}
static EpdRect shelf_header_action_rect(int action) { return (EpdRect){292 + action * 92,80,80,80}; }
static EpdRect shelf_category_title_rect(void) { return (EpdRect){36,80,244,80}; }
static EpdRect shelf_manage_rect(void) { return s_shelf_selecting?book_selection_rect(BOOK_SELECTION_SEARCH):shelf_header_action_rect(0); }
static EpdRect shelf_import_rect(void) { return s_shelf_selecting?book_selection_rect(BOOK_SELECTION_CANCEL):shelf_header_action_rect(1); }
static void draw_shelf_header_button(uint8_t *fb, EpdRect r, int action) {
    bool pressed = s_pressed_control == (action < 2 ? 114 + action : 118 + action);
    ui_fill_rect_fast(r,UI_GRAY_WHITE,fb);
    if(s_shelf_selecting)ui_draw_control_frame(fb,r,8,0x50);
    if(!action)ui_draw_icon(fb,r.x+r.width/2,r.y+r.height/2+(pressed?2:0),34,UI_ICON_SEARCH,0);
    else {
        const char *label=action==1?(s_shelf_selecting?"取消":"选择"):action==2?"管理":"导入";
        r.y+=pressed?2:0;
        shelf_label(fb,r,24,label,true);
    }
}
static EpdRect shelf_page_arrow_rect(int direction) {
    return ui_pager_rect(direction<0?UI_PAGER_PREVIOUS:UI_PAGER_NEXT);
}
static void draw_shelf_pager(uint8_t *fb, int count, int page, int pages) {
    char label[64];
    snprintf(label, sizeof(label), "%d本书 · %02d/%02d", count, page + 1, pages);
    if(s_shelf_selecting){book_selection_pager_draw(fb,label,page,pages);return;}
    int pressed=s_shelf_key_mode&&s_shelf_key_pressed?
        (s_shelf_key_pressed<0?UI_PAGER_PREVIOUS:UI_PAGER_NEXT):UI_PAGER_NONE;
    if(s_pressed_control>=116&&s_pressed_control<=119)pressed=s_pressed_control-116;
    ui_pager_draw(fb,label,page,pages,s_shelf_key_mode,pressed);
}
// 只推分页小区；按压不重绘封面，翻页在下一轮输入后执行，可被切页打断。
// Update only the pager; presses do not redraw covers. Page work follows on the next input tick and is cancellable by navigation.
static app_redraw_t shelf_key_paint(app_ctx_t *ctx) {
    s_area=ui_pager_area();
    ui_fill_rect_fast(s_area,UI_GRAY_WHITE,ctx->fb);
    draw_shelf_pager(ctx->fb,s_visible_count,ctx->leaf,leaves());
    s_shelf_key_paint_pending=true;
    return APP_REDRAW_AREA;
}
static bool shelf_main_key(app_ctx_t *ctx,int key,app_redraw_t *redraw) {
    if(s_view!=SHELF || s_clear_confirm || s_batch_confirm)return false;
    if(key==UI_KEY_2){
        s_shelf_key_mode=!s_shelf_key_mode;s_shelf_key_delta=s_shelf_key_pressed=0;
        book_shelf_categories_menu_close(&s_shelf_categories);*redraw=shelf_key_paint(ctx);return true;
    }
    if(!s_shelf_key_mode || (key!=UI_KEY_1 && key!=UI_KEY_3))return false;
    int dir=key==UI_KEY_1?-1:1;
    s_shelf_key_pressed=dir;s_shelf_key_delta+=dir;
    if(s_shelf_key_delta>8)s_shelf_key_delta=8;
    if(s_shelf_key_delta< -8)s_shelf_key_delta= -8;
    s_shelf_key_until=ctx->now_ms+35;
    *redraw=shelf_key_paint(ctx);return true;
}
static app_redraw_t shelf_turn_page(app_ctx_t *ctx, int direction);
static EpdRect import_rect(int index) { return (EpdRect){36, 230 + index * 164, 612, 136}; }
static void clean_filename(char *dst, size_t cap, const char *filename) {
    copy_text(dst, cap, filename);
    char *ext = strrchr(dst, '.');
    if (ext) *ext = 0;
    char *start = dst;
    while (*start == ' ' || *start == '\t') ++start;
    if (start != dst) memmove(dst, start, strlen(start) + 1);
    size_t len = strlen(dst);
    while (len && (dst[len - 1] == ' ' || dst[len - 1] == '\t')) dst[--len] = 0;
}

// 截断必须停在UTF8字符边界。/ Truncation must stop at a UTF8 character boundary.
static void copy_text(char* dst, size_t cap, const char* src) {
    if (!cap) return;
    size_t n = strlen(src);
    if (n >= cap) {
        n = cap - 1;
        while (n && ((unsigned char)src[n] & 0xc0) == 0x80) --n;
    }
    memcpy(dst, src, n);
    dst[n] = 0;
}
static void fit_text(char* text, int px, int width) {
    while (*text && ttf_text_width_px(ui_text_effective_px(px), text) > width) {
        size_t n = strlen(text) - 1;
        while (n && ((unsigned char)text[n] & 0xc0) == 0x80) --n;
        text[n] = 0;
    }
}

static void fit_fixed_text(char* text, int px, int width) {
    while (*text && ui_text_fixed_width_px(px, text) > width) {
        size_t n = strlen(text) - 1;
        while (n && ((unsigned char)text[n] & 0xc0) == 0x80) --n;
        text[n] = 0;
    }
}

static void reader_footer_strip_number(char *dst, size_t cap, const char *source) {
    char title[128];
    copy_text(title, sizeof(title), source ? source : "");
// name 只被读取，但下面会把字符串字面量赋给它。CI 的 sdkconfig.ci 开了
// CONFIG_COMPILER_WARN_WRITE_STRINGS，字面量因此是 const char[]，赋给 char* 会丢 const 并在
// -Werror 下报错。声明成 const 指针即可。
// name is only read from, but it is assigned a string literal below. CI's sdkconfig.ci enables
// CONFIG_COMPILER_WARN_WRITE_STRINGS, which makes literals const char[], so assigning one to a
// char* discards the qualifier and fails under -Werror. A const pointer is enough.
    const char *name = title;
    while (*name == ' ' || *name == '\t') ++name;
    char *chapter_mark = !strncmp(name, "第", strlen("第")) ? strstr(name, "章") : NULL;
    if (chapter_mark && chapter_mark - name < 24) {
        char *after = chapter_mark + strlen("章");
        while (*after == ' ' || *after == '\t' || *after == ':' || *after == '-' ||
               !strncmp(after, "：", strlen("：")) || !strncmp(after, "、", strlen("、")) ||
               !strncmp(after, "·", strlen("·"))) {
            if (*after == ' ' || *after == '\t' || *after == ':' || *after == '-') ++after;
            else after += strlen("：");
        }
        name = after;
    }
    if (!*name || !strcmp(name, "本章") || !strcmp(name, "章节")) name = "未命名章节";
    copy_text(dst, cap, name);
}
static void reader_footer_chapter_name(char *dst, size_t cap) {
    char source[128] = {0};
    bool selected = s_selected_toc < book_navigation_count() &&
        book_navigation_chapter(s_selected_toc) == s_reader_session.position.chapter &&
        book_navigation_title(s_selected_toc, source, sizeof(source)) == ESP_OK && source[0];
    if (!selected) {
        if (s_chapter_heading_title[0]) copy_text(source, sizeof(source), s_chapter_heading_title);
        else if (book_chapter_title(s_reader_session.position.chapter, source, sizeof(source)) != ESP_OK) source[0] = 0;
    }
    reader_footer_strip_number(dst, cap, source);
}
static uint32_t chapter_end(void) {
    return s_reader_session.position.chapter + 1 < book_chapter_count()
        ? book_chapter_byte_offset(s_reader_session.position.chapter + 1) : book_total_bytes();
}
static size_t reader_page_offset(size_t page) {
    size_t off = book_layout_page_start_offset(page);
    if (s_jump_offset != SIZE_MAX && page == s_jump_page && s_jump_offset > off)
        off = s_jump_offset;
    return off;
}
static unsigned percent(size_t page) {
    uint32_t total = book_total_bytes();
    if (!total) return 0;
    if (s_reader_session.position.chapter + 1 == book_chapter_count() && page + 1 == book_layout_page_count()) return 100;
    uint32_t off = book_position_bytes(book_chapter_byte_offset(s_reader_session.position.chapter), chapter_end(),
                                       reader_page_offset(page), s_text_len);
    return (unsigned)((uint64_t)off * 100 / total);
}
// 全书百分比 → 章节 + 章内偏移；目录跳转与正文进度条共用这一份换算。
// Whole-book percent to chapter and in-chapter offset; the TOC jump and the footer bar share it.
static bool reader_percent_target(int at, size_t *chapter, size_t *offset) {
    uint32_t total = book_total_bytes();
    size_t count = book_chapter_count();
    if (!total || !count) return false;
    if (at < 0) at = 0;
    if (at > 100) at = 100;
    uint32_t want = (uint32_t)((uint64_t)total * (unsigned)at / 100);
    size_t index = 0;
    for (size_t i = 1; i < count && book_chapter_byte_offset(i) <= want; ++i) index = i;
    uint32_t base = book_chapter_byte_offset(index);
    if (chapter) *chapter = index;
    if (offset) *offset = want > base ? (size_t)(want - base) : 0;
    return true;
}
// 目录底部与正文进度条必须是同一个数，否则同一位置会出现两个百分比。
// The TOC footer and the reader's progress bar must be one number, or one position shows two percents.
static int reader_book_percent(void) { return (int)percent(s_reader_session.position.page); }
// 目标位置的预览文案：优先按目录条目编号（与目录页的“N 节”一致），没有条目才退回章节序号。
// Preview label for a target position: prefer the authored TOC ordinal so it matches the list,
// falling back to the chapter ordinal when there is no navigation.
static void reader_percent_label(int at, char *out, size_t cap) {
    out[0] = 0;
    if (at < 0) at = 0;
    if (at > 100) at = 100;
    size_t chapter = 0, source = 0;
    if (!reader_percent_target(at, &chapter, &source)) return;
    const size_t nav_count = book_navigation_count();
    const uint32_t want = (uint32_t)((uint64_t)book_total_bytes() * (unsigned)at / 100);
    size_t pick = SIZE_MAX;
    for (size_t i = 0; i < nav_count; ++i) {
        size_t off = book_navigation_source_offset(i);
        if (off == SIZE_MAX || off > want || book_navigation_chapter(i) > chapter) break;
        pick = i;
    }
    char title[96] = {0};
    if (pick != SIZE_MAX && book_navigation_title(pick, title, sizeof(title)) == ESP_OK && title[0])
        snprintf(out, cap, "第 %u / %u 节 · %s", (unsigned)pick + 1, (unsigned)nav_count, title);
    else if (book_chapter_title(chapter, title, sizeof(title)) == ESP_OK && title[0])
        snprintf(out, cap, "第 %u / %u 节 · %s", (unsigned)chapter + 1,
                 (unsigned)book_chapter_count(), title);
    else snprintf(out, cap, "第 %u / %u 节", (unsigned)chapter + 1, (unsigned)book_chapter_count());
}
static pending_progress_t* pending_find(const char* path) {
    for (pending_progress_t* p = s_pending; p; p = p->next) if (!strcmp(p->path, path)) return p;
    return NULL;
}
static int layout_name(uint8_t* fb, const char* name, int y, bool draw);
static EpdRect manage_panel(void) {
    int height = layout_name(NULL, s_managed.name, 0, false) + 563;
    return (EpdRect){UI_MARGIN - 16, 190 + (876 - height) / 2, ui_content_width() + 32, height};
}
static EpdRect manage_rect(int index, int count) {
    EpdRect panel = manage_panel();
    return ui_row_rect(index, count, panel.y + panel.height - 94, 76);
}
// 六个批量按钮排成两行各三格：上排选择与重扫，下排三个需要二次确认的操作。
// Six batch controls in two rows of three: selection and rescan on top, the three actions that
// ask for confirmation below.
static EpdRect batch_rect(int id) {
    return (EpdRect){36 + (id % 3) * 208, id < 3 ? 918 : 988, 196, 58};
}
static EpdRect manage_back_rect(void) { return (EpdRect){36, 92, 44, 44}; }
static EpdRect bulk_filter_rect(int id) { return (EpdRect){36 + id * 208, 216, 196, 70}; }
static EpdRect search_rect(int id) {
    if (id < 40) {
        int width = (ui_content_width() - 54) / 10;
        return (EpdRect){UI_MARGIN + id % 10 * (width + 6), 388 + id / 10 * 100, width, 88};
    }
    if (id < 43) return ui_row_rect(id - 40, 3, 808, 80);
    return ui_bar_rect(id - 43, 2);
}
static size_t selected_count(void) {
    return book_shelf_selected_count(s_shelf, s_count);
}
static void clear_selection(void) {
    book_shelf_clear_selection(s_shelf, s_count);
}
static void toggle_selection(int index) {
    book_shelf_toggle_selection(s_shelf, s_visible_count, index);
}
static void select_page(int page) {
    book_shelf_select_page(s_shelf, s_visible_count, page, BOOK_BULK_ROWS);
}

static void search_begin(void) {
    s_search_parent = s_view;
    memcpy(s_search_draft, s_view==CATEGORIES?s_category_ui.query:s_query, sizeof(s_query));
    ui_text_edit_init(&s_search_input, s_search_draft, sizeof(s_search_draft));
    ui_keyboard_begin(&s_search_input, false);
    s_view = SEARCH;
}
static void refresh_search_matches(void) {
    for (int i = 0; i < s_count; ++i)
        s_shelf[i].search_match = book_search_match(s_shelf[i].name, s_query);
}
static void search_finish(app_ctx_t* ctx, bool apply) {
    ui_keyboard_end();
    s_view = s_search_parent;
    if (apply) {
        if(s_view==CATEGORIES){memcpy(s_category_ui.query,s_search_draft,sizeof(s_query));s_category_ui.review=false;s_category_ui.page=0;}
        else {
            memcpy(s_query, s_search_draft, sizeof(s_query));
            refresh_search_matches();
            if(!s_shelf_selecting)clear_selection();
            sort_shelf(ctx);
        }
        s_batch_message[0] = 0;
    }
    memset(s_search_draft, 0, sizeof(s_search_draft));
    ui_text_edit_init(&s_search_input, s_search_draft, sizeof(s_search_draft));
}

static bool pending_reserve(const char* path) {
    if (pending_find(path)) return true;
    pending_progress_t* p = heap_caps_malloc(sizeof(*p), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (!p) p = malloc(sizeof(*p));
    if (!p) return false;
    memset(p, 0, sizeof(*p));
    copy_text(p->path, sizeof(p->path), path);
    // 阅读页内注册；传书 HTTP 尚未启动，退出后也不注销失败项。
    // Register in reading before transfer HTTP starts; failed records survive page exit.
    p->watch = book_progress_watch_create(path);
    if (!p->watch) { free(p); return false; }
    pending_progress_t** tail = &s_pending;
    while (*tail) tail = &(*tail)->next;
    *tail = p;
    return true;
}
static bool pending_restore(const char* path, uint32_t size, book_progress_t* out) {
    pending_progress_t* pending = pending_find(path);
    if (!pending || !pending->dirty || pending->value.file_size != size) return false;
    *out = pending->value;
    return true;
}
static void pending_discard(const char* path) {
    pending_progress_t** p = &s_pending;
    while (*p) {
        if (!strcmp((*p)->path, path)) {
            pending_progress_t* old = *p;
            *p = old->next;
            book_progress_watch_destroy(old->watch);
            free(old);
            break;
        }
        p = &(*p)->next;
    }
    s_save_failed = false;
    for (pending_progress_t* item = s_pending; item; item = item->next) if (item->dirty) s_save_failed = true;
}
static void pending_mark_latest(const char* path) {
    pending_progress_t** item = &s_pending;
    while (*item && strcmp((*item)->path, path)) item = &(*item)->next;
    if (*item) {
        pending_progress_t* current = *item;
        *item = current->next;
        current->next = NULL;
        pending_progress_t** tail = &s_pending;
        while (*tail) tail = &(*tail)->next;
        *tail = current;
    }
    copy_text(s_latest_path, sizeof(s_latest_path), path);
}
static void pending_drop_invalidated(void) {
    pending_progress_t* p = s_pending;
    while (p) {
        pending_progress_t* next = p->next;
        if (book_progress_watch_invalidated(p->watch)) {
            if (p->dirty) s_pending_invalidated = true;
            pending_discard(p->path);
        }
        p = next;
    }
}
static const delete_retry_t *delete_retry_find(const char *path) {
    return book_shelf_retry_find(s_delete_retries, path);
}
static bool pending_flush(pending_progress_t* p) {
    if (!p->dirty) return true;
    if (!p->progress_saved) {
        if (book_progress_save(p->path, &p->value) != ESP_OK) return false;
        p->progress_saved = true;
    }
    if (!strcmp(p->path, s_latest_path) && book_progress_set_last_path(p->path) != ESP_OK) return false;
    p->dirty = false;
    return true;
}
static void retry_progress(void) {
    s_save_failed = false;
    bool had_dirty = false;
    pending_progress_t* p = s_pending;
    while (p) {
        pending_progress_t* next = p->next;
        bool dirty = p->dirty;
        had_dirty |= dirty;
        if (s_text && !strcmp(p->path, s_reader_session.book.path)) { p = next; continue; }
        if (!pending_flush(p)) s_save_failed = true;
        else if (strcmp(p->path, s_reader_session.book.path)) pending_discard(p->path);
        else if (dirty) s_unsaved = 0;
        p = next;
    }
    if (s_text && (s_unsaved || had_dirty)) save_progress();
    s_save_failed = false;
    for (pending_progress_t* item = s_pending; item; item = item->next) if (item->dirty) s_save_failed = true;
}

/* ---- 管理详情 / Management details ---- */
static int layout_name(uint8_t* fb, const char* name, int y, bool draw) {
    const char* at = name;
    while (*at) {
        char line[256];
        size_t used = 0;
        while (at[used]) {
            unsigned char first = (unsigned char)at[used];
            size_t n = first < 0x80 ? 1 : first < 0xe0 ? 2 : first < 0xf0 ? 3 : 4;
            size_t remain = strlen(at + used);
            if (n > remain) n = 1;
            if (used + n >= sizeof(line)) break;
            memcpy(line + used, at + used, n);
            line[used + n] = 0;
            if (ttf_text_width_px(UI_PX_CAPTION, line) > ui_content_width() && used) break;
            used += n;
        }
        if (!used) break;
        line[used] = 0;
        if (draw) ui_text(fb, UI_MARGIN, y, UI_PX_CAPTION, line, EPD_DRAW_ALIGN_LEFT, false);
        y += 36;
        at += used;
    }
    return y;
}
static int draw_wrapped_name(uint8_t* fb, const char* name, int y) {
    return layout_name(fb, name, y, true);
}
static void draw_manage(uint8_t* fb) {
    EpdRect panel = manage_panel();
    ui_fill_round_rect(fb, panel, UI_BTN_RADIUS, UI_GRAY_WHITE);
    ui_draw_control_frame(fb, panel, UI_BTN_RADIUS, UI_GRAY_BLACK);
    ui_text(fb, UI_MARGIN, panel.y + 18, UI_PX_BODY,
            s_clear_confirm ? (s_delete_confirm ? "确认删除文件？" : "确认清除进度？") : "图书详情", EPD_DRAW_ALIGN_LEFT, false);
    int y = draw_wrapped_name(fb, s_managed.name, panel.y + 74) + 12;
    char info[96];
    snprintf(info, sizeof(info), "%s · %s · %.2f MB", s_managed.is_flash ? "内置存储" : "TF 卡",
             strrchr(s_managed.name, '.') ? strrchr(s_managed.name, '.') + 1 : "", s_managed.size / 1048576.0);
    ui_text(fb, UI_MARGIN, y, UI_PX_CAPTION, info, EPD_DRAW_ALIGN_LEFT, false);
    snprintf(info, sizeof(info), s_managed.has_progress ? "阅读进度 %u%%" : "尚无阅读进度", s_managed.pct);
    ui_text(fb, UI_MARGIN, y + 44, UI_PX_CAPTION, info, EPD_DRAW_ALIGN_LEFT, false);
    ui_text(fb, UI_MARGIN, y + 88, UI_PX_CAPTION,
            s_managed.is_flash ? "/flash/books" : !strncmp(s_managed.path, "/sdcard/book/", 13) ? "/sdcard/book" : app_settings_books_dir(),
            EPD_DRAW_ALIGN_LEFT, false);
    if (s_manage_message[0]) ui_text(fb, UI_MARGIN, panel.y + panel.height - 334, UI_PX_CAPTION, s_manage_message, EPD_DRAW_ALIGN_LEFT, false);
    if (s_clear_confirm) {
        ui_text(fb, UI_MARGIN, panel.y + panel.height - 142, UI_PX_CAPTION, s_delete_confirm ? "删除后文件无法恢复" : "仅清阅读进度，保留图书文件", EPD_DRAW_ALIGN_LEFT, false);
        draw_control(fb, manage_rect(0, 2), "取消", 300);
        draw_control(fb, manage_rect(1, 2), s_delete_confirm ? "确认删除" : "确认清除", 301);
    } else if (s_file_removed) {
        draw_control(fb, manage_rect(0, 2), "关闭", 400);
        draw_control(fb, manage_rect(1, 2), "重试清理", 403);
    } else {
        char category_label[128];
        snprintf(category_label,sizeof(category_label),"分类：%s",shelf_category_name(book_shelf_categories_index(&s_shelf_categories,book_collection_assignment(&s_collection,s_managed.path))));
        fit_text(category_label,20,ui_content_width()-24);
        draw_control(fb, (EpdRect){UI_MARGIN, panel.y + panel.height - 258, ui_content_width(), 58}, category_label, 407);
        draw_control(fb, (EpdRect){UI_MARGIN, panel.y + panel.height - 190, ui_content_width(), 68}, "前往文件管理重命名", 404);
        draw_control(fb, manage_rect(0, 4), "关闭", 400);
        draw_control(fb, manage_rect(1, 4), s_managed.favorite ? "取消收藏" : "收藏", 406);
        draw_control(fb, manage_rect(2, 4), "清进度", 401);
        draw_control(fb, manage_rect(3, 4), "删除文件", 402);
    }
}

static void editor_start(void) {
    free(s_editor_cover);
    bool pending = false;
    s_editor_cover = load_cover_gray(s_managed.path, s_managed.name, "", false, &pending);
    copy_text(s_editor_title, sizeof(s_editor_title), s_managed.name);
    ui_text_edit_init(&s_editor_input, s_editor_title, sizeof(s_editor_title));
    s_editor_notice[0] = 0;
    ui_keyboard_begin(&s_editor_input, false);
    s_view = EDIT;
}



static void draw_editor(uint8_t* fb) {
    ui_clear_page(fb);
    ui_nav_status(fb);
    ui_nav_back(fb, 36, 79);
    ui_text_vc(fb, UI_LOCK_WIDTH / 2, 107, 31, "编辑书籍", EPD_DRAW_ALIGN_CENTER, false);
    ui_text_vc(fb, 646, 107, 26, "完成", EPD_DRAW_ALIGN_RIGHT, false);
    ui_hairline(fb, 133, 36, 612, UI_GRAY_LIGHT);
    EpdRect cover = {36, 175, 112, 152};
    ui_fill_rect_fast(cover, UI_GRAY_LIGHT, fb);
    if (s_editor_cover) {

        for (int y = 0; y < cover.height; ++y) {
            for (int x = 0; x < cover.width; ++x) {
                const uint8_t tone = ui_contrast_gray(
                    s_editor_cover[(y * BOOK_COVER_H / cover.height) * BOOK_COVER_W +
                                   x * BOOK_COVER_W / cover.width]);
                epd_draw_pixel(cover.x + x, cover.y + y,
                    ui_image_dither_gray(tone, cover.x + x, cover.y + y), fb);
            }
        }
    } else {
        char title[80]; copy_text(title, sizeof(title), s_managed.name);
        fit_text(title, 22, cover.width - 16);
        ui_text_vc(fb, cover.x + cover.width / 2, cover.y + cover.height / 2, 22, title, EPD_DRAW_ALIGN_CENTER, false);
    }
    ui_draw_round_rect(fb, cover, 0, UI_GRAY_BLACK);
    char title[128]; copy_text(title, sizeof(title), s_managed.name);
    fit_text(title, 30, 460);
    ui_text(fb, 174, 192, 30, title, EPD_DRAW_ALIGN_LEFT, false);
    ui_text(fb, 174, 244, 22, "只修改书架显示名称", EPD_DRAW_ALIGN_LEFT, false);
    ui_hairline(fb, 356, 36, 612, UI_GRAY_LIGHT);
    ui_text(fb, 36, 378, 24, "书名", EPD_DRAW_ALIGN_LEFT, false);
    EpdRect field = {36, 424, 612, 83};
    ui_text_input_draw(fb, &s_editor_input, field, 31, false, NULL);
    if (s_editor_notice[0]) ui_text_fixed_vc(fb, 36, 530, 22, s_editor_notice, EPD_DRAW_ALIGN_LEFT, false);
    ui_keyboard_draw(fb, 560);
}
static void draw_search(uint8_t* fb) {
    ui_clear_page(fb); ui_nav_status(fb);
    ui_nav_back(fb, 36, 79);
    ui_text_vc(fb, 342, 107, 34, "搜索图书", EPD_DRAW_ALIGN_CENTER, false);
    ui_text_vc(fb, 646, 107, 25, "应用", EPD_DRAW_ALIGN_RIGHT, false);
    ui_text_input_draw(fb, &s_search_input, (EpdRect){UI_MARGIN, 200, ui_content_width(), 88}, UI_PX_BODY, false, NULL);
    ui_text_fixed_vc(fb, 36, 337, 23, "可输入中文、完整拼音、首字母或英文", EPD_DRAW_ALIGN_LEFT, false);
    ui_draw_button(fb, (EpdRect){36, 392, 180, 66}, "清空", false);
    ui_keyboard_draw(fb, 560);
}
static void draw_import(uint8_t* fb) {
    ui_clear_page(fb);
    ui_nav_status(fb);
    ui_nav_back(fb, 36, 79);
    ui_text_vc(fb, 342, 107, 34, "导入图书", EPD_DRAW_ALIGN_CENTER, false);
    ui_text(fb, 36, 166, 20, "选择导入方式", EPD_DRAW_ALIGN_LEFT, false);
    ui_hairline(fb, 207, 36, 612, UI_GRAY_LIGHT);
    static const char *titles[] = {"浏览 TF 卡", "WiFi 传书", "热点传书", "USB 读卡"};
    static const char *details[] = {
        "查看目录，打开或管理已有图书", "手机与墨水屏连接同一网络",
        "连接 mogo 热点后上传图书", "连接电脑，把图书放入 books 文件夹"
    };
    for (int i = 0; i < 4; ++i) {
        EpdRect r = import_rect(i);
        ui_fill_round_rect(fb, r, 22, UI_GRAY_WHITE);
        ui_draw_control_frame(fb, r, 22, 0xb0);
        ui_text(fb, r.x + 24, r.y + 25, 27, titles[i], EPD_DRAW_ALIGN_LEFT, false);
        ui_text(fb, r.x + 24, r.y + 78, 19, details[i], EPD_DRAW_ALIGN_LEFT, false);
        ui_text_vc(fb, r.x + r.width - 24, r.y + r.height / 2, 26, "›", EPD_DRAW_ALIGN_RIGHT, false);
    }
    ui_text(fb, 36, 925, 19, "图书放入 TF 卡的 books 文件夹后会自动出现在书架。",
            EPD_DRAW_ALIGN_LEFT, false);
    ui_nav_draw(fb, 1);
}
static void draw_batch_confirmation(uint8_t* fb) {
    if (!s_batch_confirm) return;
    EpdRect panel = {54, 406, 576, 330};
    ui_fill_round_rect(fb, panel, 25, UI_GRAY_WHITE);
    ui_draw_control_frame(fb, panel, 25, 0x48);
    char title[96];
    const char* verb = s_batch_kind == BATCH_DELETE ? "删除" :
                       s_batch_kind == BATCH_CLEAR ? "清除进度：" : "从书架移除";
    const char* note = s_batch_kind == BATCH_DELETE ? "删除文件不可撤销，失败项可重试" :
                       s_batch_kind == BATCH_CLEAR ? "仅清阅读进度，所有文件保留" :
                       "只移出书架，文件与阅读进度都保留";
    snprintf(title, sizeof(title), "%s %u 本图书？", verb, (unsigned)selected_count());
    ui_text_vc(fb, 342, 477, 28, title, EPD_DRAW_ALIGN_CENTER, false);
    ui_text_vc(fb, 342, 541, 19, note, EPD_DRAW_ALIGN_CENTER, false);
    draw_control(fb, ui_row_rect(0, 2, 620, UI_BTN_H), "取消", 600);
    draw_control(fb, ui_row_rect(1, 2, 620, UI_BTN_H), "确认", 601);
}

static EpdRect bulk_nav_rect(int i) { return (EpdRect){36 + i * 208, UI_BAR_TOP, 196, UI_BAR_H}; }
static int bulk_nav_hit(int x, int y) {
    for (int i=0;i<3;++i) if(ui_rect_hit(bulk_nav_rect(i),x,y)) return i;
    return -1;
}
static void draw_bulk(uint8_t* fb, int leaf) {
    ui_clear_page(fb);
    ui_nav_status(fb);
    ui_nav_back(fb, 36, 79);
    ui_text_vc(fb, 342, 107, 34, "管理书架", EPD_DRAW_ALIGN_CENTER, false);
    ui_draw_icon(fb, 620, 107, 38, UI_ICON_FOLDER_INPUT, UI_GRAY_BLACK);
    char summary[96];
    snprintf(summary, sizeof(summary), "%d 本图书 · 已选 %u 本", s_visible_count, (unsigned)selected_count());
    ui_text(fb, 36, 166, 20, s_batch_message[0] ? s_batch_message : summary, EPD_DRAW_ALIGN_LEFT, false);
    ui_hairline(fb, 200, 36, 612, 0x70);
    const char *filters[] = {s_filter == 0 ? "全部来源" : s_filter == 1 ? "TF 卡" : "内置",
                             shelf_query().recent_sort ? "按最近" : "按名称", s_query[0] ? "搜索中" : "搜索"};
    for (int i = 0; i < 3; ++i) {
        EpdRect r = bulk_filter_rect(i);
        ui_fill_round_rect(fb, r, 20, UI_GRAY_WHITE);
        ui_draw_control_frame(fb, r, 20, 0x70);
        ui_text_vc(fb, r.x + r.width / 2, r.y + 35, 21, filters[i], EPD_DRAW_ALIGN_CENTER, false);
    }
    for (int row = 0; row < BOOK_BULK_ROWS; ++row) {
        int index = leaf * BOOK_BULK_ROWS + row;
        if (index >= s_visible_count) break;
        EpdRect r = row_rect(row);
        bool selected = s_shelf[index].selected;
        ui_fill_round_rect(fb, r, 18, selected ? 0xd0 : UI_GRAY_WHITE);
        ui_draw_control_frame(fb, r, 18, selected ? 0x60 : 0x70);
        char title[128]; copy_text(title, sizeof(title), s_shelf[index].name);
        fit_text(title, 26, 495);
        ui_text_vc(fb, r.x + 22, r.y + 31, 26, title, EPD_DRAW_ALIGN_LEFT, false);
        char detail[48];
        snprintf(detail, sizeof(detail), "%s · %s", s_shelf[index].is_flash ? "内置" : "TF 卡",
                 s_shelf[index].has_progress ? "已有阅读进度" : "未开始阅读");
        ui_text_vc(fb, r.x + 22, r.y + 62, 20, detail, EPD_DRAW_ALIGN_LEFT, false);
        epd_draw_circle(r.x + r.width - 37, r.y + 42, 12, UI_GRAY_BLACK, fb);
        if (selected) epd_fill_circle(r.x + r.width - 37, r.y + 42, 7, UI_GRAY_BLACK, fb);
    }
    if (!s_visible_count)
        ui_text_vc(fb, 342, 570, 26, s_message[0] ? s_message : "当前筛选没有图书", EPD_DRAW_ALIGN_CENTER, false);
    // 三列格宽只有 193 像素，标签控制在四字以内，放大系统字号也不会顶出按钮。
    // A three-column cell is 193 px wide, so labels stay within four characters and still fit
    // once the system font scale is turned up.
    static const char *labels[] = {"本页全选", "清除勾选", "重新扫描",
                                   "删除所选", "清进度", "移出书架"};
    for (int i = 0; i < 6; ++i) {
        EpdRect r = batch_rect(i);
        ui_fill_round_rect(fb, r, 18, UI_GRAY_WHITE);
        ui_draw_control_frame(fb, r, 18, 0x70);
        ui_text_vc(fb, r.x + r.width / 2, r.y + r.height / 2, 20, labels[i], EPD_DRAW_ALIGN_CENTER, false);
    }
    const char *nav[] = {"上一页", "完成", "下一页"};
    char page[48];
    snprintf(page, sizeof(page), "%d / %d 页", leaf + 1, leaves());
    ui_text_vc(fb, 342, 1072, 20, page, EPD_DRAW_ALIGN_CENTER, false);
    for (int i = 0; i < 3; ++i) {
        EpdRect r = bulk_nav_rect(i);
        bool available = i == 1 || (i == 0 ? leaf > 0 : leaf + 1 < leaves());
        ui_fill_round_rect(fb, r, 20, i == 1 ? 0x20 : UI_GRAY_WHITE);
        ui_draw_control_frame(fb, r, 20, available ? 0x70 : 0xb0);
        ui_text_vc(fb, r.x + r.width / 2, r.y + r.height / 2, 25, nav[i], EPD_DRAW_ALIGN_CENTER, i == 1);
    }
    draw_batch_confirmation(fb);
}
static void save_progress(void) {
    if (!s_text || !s_reader_session.book.path[0] || !book_layout_page_count()) return;
    bool was_failed = s_save_failed;
    pending_progress_t* pending = pending_find(s_reader_session.book.path);
    if (!pending) { s_save_failed = true; return; }
    book_progress_t p = {
        .file_size = s_reader_session.book.file_size, .chapter = (uint16_t)s_reader_session.position.chapter,
        .byte_off = (uint32_t)reader_page_offset(s_reader_session.position.page),
        .px = (uint8_t)s_px, .pct = (uint8_t)percent(s_reader_session.position.page),
        .last_open_s = 0,
    };
    if (!pending->dirty || pending->value.file_size != p.file_size || pending->value.chapter != p.chapter ||
        pending->value.byte_off != p.byte_off || pending->value.px != p.px || pending->value.pct != p.pct) {
        pending->value = p;
        pending->progress_saved = false;
    }
    pending->dirty = true;
    if (pending_flush(pending)) {
        s_unsaved = 0;
        s_save_failed = false;
        for (pending_progress_t* item = s_pending; item; item = item->next) if (item->dirty) s_save_failed = true;
    }
    else { if (!s_unsaved) s_unsaved = 1; s_save_failed = true; }
    if (was_failed != s_save_failed) invalidate_prep();
}
static void invalidate_prep(void) { reader_prefetch_invalidate(); }

// 目录使用系统字体；切换前等待预渲染结束，回正文时恢复阅读字体与字形缓存。
// The directory uses the system face; join painting before switching and restore the reader face on return.
static void set_reader_view(book_view_t view) {
    reader_indent_hold_cancel();
    lock_draw();
    invalidate_prep();
    if (view == READING) app_font_activate_reading();
    else app_font_activate_system();
    s_view = view;
    // 离开正文时丢弃未到期的中键单击，回来不再补发。
    // Drop an unexpired middle click when leaving the body so it never replays on return.
    if (view != READING) middle_pending_clear();
    unlock_draw();
}

/* ---- 阅读工具层 / Reader overlays ---- */
static void bookmark_key(const char* path, char key[11]) {
    uint32_t hash = UINT32_C(2166136261);
    for (const unsigned char* p = (const unsigned char*)path; *p; ++p)
        hash = (hash ^ *p) * UINT32_C(16777619);
    snprintf(key, 11, "m_%08lx", (unsigned long)hash);
}
// 收藏按完整路径持久化，与章节书签分别存放。/ Persist whole-book favorites separately from page bookmarks.
static void favorite_key(const char *path, char key[11]) {
    uint32_t hash = UINT32_C(2166136261);
    for (const unsigned char *p = (const unsigned char *)path; *p; ++p)
        hash = (hash ^ *p) * UINT32_C(16777619);
    snprintf(key, 11, "f_%08lx", (unsigned long)hash);
}
static bool favorite_read_handle(nvs_handle_t h, const char *path) {
    uint8_t value = 0;
    char key[11]; favorite_key(path, key);
    return nvs_get_u8(h, key, &value) == ESP_OK && value == 1;
}
static bool favorite_save(const char *path, bool value) {
    nvs_handle_t h;
    if (nvs_open("rp_favs", NVS_READWRITE, &h) != ESP_OK) return false;
    char key[11]; favorite_key(path, key);
    esp_err_t err = value ? nvs_set_u8(h, key, 1) : nvs_erase_key(h, key);
    if (err == ESP_ERR_NVS_NOT_FOUND && !value) err = ESP_OK;
    if (err == ESP_OK) err = nvs_commit(h);
    nvs_close(h);
    return err == ESP_OK;
}

// 从书架移除只隐藏条目，不动文件也不清进度；再次打开这本书会重新上书架。
// Removing from the shelf hides the entry only: no file or progress is touched, and opening the
// book again puts it back. 值为完整路径，命中的同键记录属于别的书时拒绝覆盖。
// The value is the full path, so a colliding key owned by a different book is never overwritten.
#define SHELF_HIDDEN_NS "rp_shelf"
static void shelf_hidden_key(const char *path, char key[11]) {
    uint32_t hash = UINT32_C(2166136261);
    for (const unsigned char *p = (const unsigned char *)path; *p; ++p)
        hash = (hash ^ *p) * UINT32_C(16777619);
    snprintf(key, 11, "h_%08lx", (unsigned long)hash);
}
static bool shelf_hidden_read_handle(nvs_handle_t h, const char *path) {
    char stored[BOOK_STORE_PATH_MAX];
    char key[11];
    shelf_hidden_key(path, key);
    size_t len = sizeof(stored);
    return nvs_get_str(h, key, stored, &len) == ESP_OK && strcmp(stored, path) == 0;
}
static bool shelf_hidden_load(const char *path) {
    nvs_handle_t h;
    if (nvs_open(SHELF_HIDDEN_NS, NVS_READONLY, &h) != ESP_OK) return false;
    bool hidden = shelf_hidden_read_handle(h, path);
    nvs_close(h);
    return hidden;
}
static bool shelf_hidden_save(const char *path, bool hidden) {
    nvs_handle_t h;
    if (nvs_open(SHELF_HIDDEN_NS, NVS_READWRITE, &h) != ESP_OK) return false;
    char key[11];
    shelf_hidden_key(path, key);
    char stored[BOOK_STORE_PATH_MAX];
    size_t len = sizeof(stored);
    esp_err_t existing = nvs_get_str(h, key, stored, &len);
    esp_err_t err = existing == ESP_ERR_NVS_NOT_FOUND ? ESP_OK : existing;
    if (existing == ESP_OK && strcmp(stored, path) != 0) err = ESP_ERR_INVALID_STATE;
    // 恢复上架也校验路径，避免擦掉碰撞键属于另一书的隐藏记录。
    // Check ownership when unhiding too, so a collision cannot erase a different book removal.
    if (err == ESP_OK && hidden) err = nvs_set_str(h, key, path);
    else if (err == ESP_OK) {
        err = nvs_erase_key(h, key);
        if (err == ESP_ERR_NVS_NOT_FOUND) err = ESP_OK;
    }
    if (err == ESP_OK) err = nvs_commit(h);
    nvs_close(h);
    if(err==ESP_OK)book_lock_collage_invalidate();
    return err == ESP_OK;
}
static bool bookmark_read_handle(nvs_handle_t h, reader_bookmarks_t* marks) {
    if (!marks || !s_reader_session.book.path[0]) return false;
    memset(marks, 0, sizeof(*marks));
    char key[11]; bookmark_key(s_reader_session.book.path, key);
    size_t size = 0;
    if (nvs_get_blob(h, key, NULL, &size) != ESP_OK) return false;
    if (size == sizeof(legacy_reader_bookmark_t)) {
        legacy_reader_bookmark_t old;
        size = sizeof(old);
        if (nvs_get_blob(h, key, &old, &size) != ESP_OK ||
            old.magic != UINT32_C(0x52504d31) || old.file_size != s_reader_session.book.file_size ||
            strcmp(old.path, s_reader_session.book.path)) return false;
        marks->magic = UINT32_C(0x52504d32);
        marks->file_size = old.file_size;
        marks->count = 1;
        copy_text(marks->path, sizeof(marks->path), old.path);
        marks->entries[0] = (reader_bookmark_entry_t){
            .chapter = old.chapter, .byte_off = old.byte_off,
        };
        return true;
    }
    if (size != sizeof(*marks) || nvs_get_blob(h, key, marks, &size) != ESP_OK) return false;
    return marks->magic == UINT32_C(0x52504d32) && marks->file_size == s_reader_session.book.file_size &&
        marks->count <= BOOKMARK_MAX && !strcmp(marks->path, s_reader_session.book.path);
}

static bool bookmark_load(reader_bookmarks_t* marks) {
    if (!marks || !s_reader_session.book.path[0]) return false;
    nvs_handle_t h;
    if (nvs_open("rp_marks", NVS_READONLY, &h) != ESP_OK) return false;
    bool ok = bookmark_read_handle(h, marks);
    nvs_close(h);
    return ok;
}
static bool bookmark_is_current(void) {
    reader_bookmarks_t marks;
    if (!bookmark_load(&marks)) return false;
    size_t offset = book_layout_page_start_offset(s_reader_session.position.page);
    for (size_t i = 0; i < marks.count; ++i)
        if (marks.entries[i].chapter == s_reader_session.position.chapter && marks.entries[i].byte_off == offset) return true;
    return false;
}
static size_t bookmark_count(void) {
    reader_bookmarks_t marks;
    return bookmark_load(&marks) ? marks.count : 0;
}
static int bookmark_row_step(void) {
    int scale=app_settings_system_font_size();
    if(scale<100)scale=100;
    if(scale>200)scale=200;
    return 82+(scale-100)/2;
}
static int bookmark_rows(void) { return (s_bookmark_edit?424:492)/bookmark_row_step(); }
static EpdRect bookmark_row_rect(int row) {
    int step=bookmark_row_step();
    return (EpdRect){36,656+row*step,612,step-10};
}
static int bookmark_pages(size_t count) {
    return count ? ((int)count + bookmark_rows() - 1) / bookmark_rows() : 1;
}
static unsigned bookmark_selected_count(void) {
    unsigned count = 0;
    for (unsigned i = 0; i < BOOKMARK_MAX; ++i) if (s_bookmark_selected & (UINT32_C(1) << i)) ++count;
    return count;
}
static void bookmark_compact(reader_bookmarks_t* marks, uint32_t selected) {
    size_t write = 0;
    for (size_t i = 0; i < marks->count; ++i)
        if (!(selected & (UINT32_C(1) << i))) marks->entries[write++] = marks->entries[i];
    marks->count = (uint16_t)write;
}
static bool bookmark_delete_selected(void) {
    if (!s_bookmark_selected || !s_reader_session.book.path[0]) return false;
    nvs_handle_t h;
    if (nvs_open("rp_marks", NVS_READWRITE, &h) != ESP_OK) return false;
    reader_bookmarks_t marks;
    bool valid = bookmark_read_handle(h, &marks);
    esp_err_t err = valid ? ESP_OK : ESP_ERR_NOT_FOUND;
    if (valid) {
        bookmark_compact(&marks, s_bookmark_selected);
        char key[11]; bookmark_key(s_reader_session.book.path, key);
        err = marks.count ? nvs_set_blob(h, key, &marks, sizeof(marks)) : nvs_erase_key(h, key);
        if (err == ESP_OK) err = nvs_commit(h);
    }
    nvs_close(h);
    if (err != ESP_OK) return false;
    s_bookmark_selected = 0;
    s_bookmark_page = 0;
    s_bookmark_edit = marks.count != 0;
    return true;
}
static bool bookmark_toggle(void) {
    if (!s_reader_session.book.path[0]) return false;
    nvs_handle_t h;
    if (nvs_open("rp_marks", NVS_READWRITE, &h) != ESP_OK) return false;
    char key[11]; bookmark_key(s_reader_session.book.path, key);
    reader_bookmarks_t marks;
    bool valid = bookmark_read_handle(h, &marks);
    if (!valid) {
        memset(&marks, 0, sizeof(marks));
        marks.magic = UINT32_C(0x52504d32);
        marks.file_size = s_reader_session.book.file_size;
        copy_text(marks.path, sizeof(marks.path), s_reader_session.book.path);
    }
    uint32_t offset = (uint32_t)book_layout_page_start_offset(s_reader_session.position.page);
    size_t found = marks.count;
    for (size_t i = 0; i < marks.count; ++i)
        if (marks.entries[i].chapter == s_reader_session.position.chapter && marks.entries[i].byte_off == offset) { found = i; break; }
    bool remove = found < marks.count;
    if (remove) {
        memmove(&marks.entries[found], &marks.entries[found + 1],
                (marks.count - found - 1) * sizeof(marks.entries[0]));
        --marks.count;
    } else {
        if (marks.count == BOOKMARK_MAX) {
            memmove(&marks.entries[0], &marks.entries[1],
                    (BOOKMARK_MAX - 1) * sizeof(marks.entries[0]));
            --marks.count;
        }
        time_t now = time(NULL);
        marks.entries[marks.count++] = (reader_bookmark_entry_t){
            .chapter = (uint16_t)s_reader_session.position.chapter, .byte_off = offset,
            .saved_s = now >= 1704067200 ? (uint32_t)now : 0,
        };
    }
    esp_err_t err = marks.count ? nvs_set_blob(h, key, &marks, sizeof(marks)) : nvs_erase_key(h, key);
    if (err == ESP_ERR_NVS_NOT_FOUND && !marks.count) err = ESP_OK;
    if (err == ESP_OK) err = nvs_commit(h);
    nvs_close(h);
    if (err == ESP_OK) {
        copy_text(s_reader_notice, sizeof(s_reader_notice), remove ? "书签已移除" : "书签已添加");
        s_reader_notice_until = esp_timer_get_time() / 1000 + 1600;
    } else {
        copy_text(s_reader_notice, sizeof(s_reader_notice), "书签保存失败");
        s_reader_notice_until = esp_timer_get_time() / 1000 + 2200;
    }
    return err == ESP_OK;
}

static void draw_reader_tool_icon(uint8_t* fb, int cx, int cy, int kind, bool selected) {
    // 阅读工具栏图标全部来自 Lucide；已保存的书签用带勾的一档区分未保存，
    // 全屏按钮随当前状态在进入/退出两枚图标之间切换。
    // Reader toolbar icons all come from Lucide; a saved bookmark uses the check
    // variant, and the full-screen button follows the current state.
    static const ui_icon_t tools[] = {
        UI_ICON_LIST,          // 0 目录
        UI_ICON_BOOKMARK,      // 1 书签
        UI_ICON_CHART_COLUMN,  // 2 阅读统计
        UI_ICON_BOOK_OPEN,     // 3 阅读设置 / Reading settings
        UI_ICON_TYPE,          // 4 字体设置
        UI_ICON_MAXIMIZE_2,    // 5 全屏 / Full screen
    };
    if (kind < 0 || kind >= (int)(sizeof(tools) / sizeof(tools[0]))) return;
    const ui_icon_t icon = (kind == 1 && selected) ? UI_ICON_BOOKMARK_CHECK
                         : (kind == 5 && s_reader_fullscreen) ? UI_ICON_MINIMIZE_2
                         : tools[kind];
    ui_draw_icon(fb, cx, cy, READER_TOOL_ICON_PX, icon, 0x38);
}

// 单行辅助文字保持在当前控件内；本机只使用当前活动字库，按原轮廓重新光栅化。
// Keep one-line UI text inside its slot, rasterizing native outlines in the existing active font.
static void reader_panel_text_vc(uint8_t *fb,int x,int cy,int px,const char *text,
                                  enum EpdFontFlags align,bool inverted) {
    int width=align==EPD_DRAW_ALIGN_LEFT?642-x:align==EPD_DRAW_ALIGN_RIGHT?x-42:
              2*(x<342?x-42:642-x);
    int left=align==EPD_DRAW_ALIGN_LEFT?x:align==EPD_DRAW_ALIGN_RIGHT?x-width:x-width/2;
    ui_text_box(fb,(EpdRect){left,cy-22,width,44},px,text,align,inverted);
}
static void reader_panel_text(uint8_t *fb,int x,int y,int px,const char *text,
                               enum EpdFontFlags align,bool inverted) {
    int height=ui_text_effective_px(px);if(height>44)height=44;
    reader_panel_text_vc(fb,x,y+height/2,px,text,align,inverted);
}

static void draw_sheet(uint8_t* fb, int top, const char* title) {
    EpdRect sheet = {0, top, UI_LOCK_WIDTH, UI_LOCK_HEIGHT - top + 28};
    ui_fill_round_rect(fb, sheet, 28, UI_GRAY_WHITE);
    ui_fill_rect_fast((EpdRect){0, top + 28, UI_LOCK_WIDTH, UI_LOCK_HEIGHT - top - 28}, UI_GRAY_WHITE, fb);
    ui_draw_control_frame(fb, sheet, 28, 0x58);
    ui_fill_round_rect(fb, (EpdRect){292, top + 14, 100, 7}, 3, 0x48);
    ui_text_box(fb,(EpdRect){110,top+26,464,66},30,title,EPD_DRAW_ALIGN_CENTER,false);
}

// 二级面板共用 AnimateIcons circle-chevron-left，含圆环与箭头，避免叠加手画框线。
// Secondary panels share the complete AnimateIcons circle-chevron-left without an extra hand-drawn ring.
static void draw_sheet_back(uint8_t* fb, int top) {
    ui_click_feedback_register(fb,(EpdRect){32,top+25,64,64},UI_CLICK_BACK,UI_ICON_ANIMATE_CIRCLE_CHEVRON_LEFT);
    ui_draw_icon(fb,64,top+57,READER_SHEET_BACK_PX,UI_ICON_ANIMATE_CIRCLE_CHEVRON_LEFT,0x30);
}

static void draw_pill_slider(uint8_t* fb, EpdRect r, const char* left, const char* right,
                             const char* value, int index, int count, int left_px, int right_px) {
    ui_fill_round_rect(fb, r, r.height / 2, 0xb8);
    ui_draw_control_frame(fb, r, r.height / 2, 0x50);
    int cy = r.y + r.height / 2;
    ui_text_box(fb,(EpdRect){r.x+12,cy-26,40,52},left_px,left,EPD_DRAW_ALIGN_CENTER,false);
    ui_text_box(fb,(EpdRect){r.x+r.width-52,cy-26,40,52},right_px,right,EPD_DRAW_ALIGN_CENTER,false);
    int span = r.width - 128;
    int cx = r.x + 64 + (count > 1 ? span * index / (count - 1) : 0);
    EpdRect track = {r.x + 59, cy - 3, r.width - 118, 6};
    ui_fill_round_rect(fb, track, 3, 0x78);
    track.width = cx - track.x;
    if (track.width > 0) ui_fill_round_rect(fb, track, 3, 0x40);
    epd_fill_circle(cx, cy, 37, UI_GRAY_WHITE, fb);
    epd_draw_circle(cx, cy, 37, UI_GRAY_BLACK, fb);
    ui_text_box(fb,(EpdRect){cx-31,cy-28,62,56},22,value,EPD_DRAW_ALIGN_CENTER,false);
}

static EpdRect reader_slider_refresh_rect(int slider) {
    EpdRect r = reader_slider_rect(slider);
    // The knob extends four pixels outside the pill. Include that margin so
    // moving it restores the old white footprint and the rail beneath it.
    r.x -= 8; r.y -= 8; r.width += 16; r.height += 16;
    return r;
}

static const char* font_friendly_name(const ttf_font_item_t* item) {
    return item ? ttf_font_localized_name(item->name) : "";
}

static void draw_font_name_with_current_face(uint8_t* fb, int x, int y, const char* name, int width) {
    int above = 0, below = 0;
    int px=ui_text_effective_px(25);
    while (px>1 && ttf_text_width_px(px,name)>width) --px;
    ttf_measure_line_px(px, name, &above, &below);
    ttf_draw_text_px(fb, x, y + (above - below) / 2, px, name,
                     EPD_DRAW_ALIGN_CENTER, UI_INK_BLACK, UI_INK_WHITE);
}

// 用户确认双列：字体/字重在第一行，书内字体/排版在第二行；绘制与命中共用矩形。
// Approved paired rows: face/weight, then embedded faces/layout; paint and touch share rectangles.
// 放大文字时向上展开面板，保持双列与底部完成键的位置；命中也使用这些矩形。
// Expand sheets upward for larger UI text, retaining paired columns and the Done button; hits share these rectangles.
static int reader_ui_extra(int maximum) {
    int scale = app_settings_system_font_size();
    if (scale < 100) scale = 100;
    if (scale > 200) scale = 200;
    return (scale-100)*maximum/100;
}
static int reader_font_top(void) { return 580-reader_ui_extra(112); }
static EpdRect reader_font_card(int index) {
    int extra=reader_ui_extra(48), footer=reader_ui_extra(16);
    if (index>=4) return (EpdRect){index==4?36:350,1054-footer,298,72+footer};
    int row=index/2;
    return (EpdRect){index%2?350:36,796+row*132-(2-row)*extra-footer,
                     298,(row?110:116)+extra};
}
#define s_font_card reader_font_card(0)
#define s_weight_card reader_font_card(1)
#define s_bookfont_card reader_font_card(2)
#define s_layout_card reader_font_card(3)
#define s_rule_card reader_font_card(4)
#define s_antialias_card reader_font_card(5)
static void draw_reading_switch(uint8_t *fb, EpdRect track, bool on);
static const EpdRect s_rule_offset_up = {36, 980, 190, 88};
static const EpdRect s_rule_offset_reset = {246, 980, 192, 88};
static const EpdRect s_rule_offset_down = {458, 980, 190, 88};

// 阅读背景选择面板已随 2026-10-25 入口迁移删除；本文件只保留背景的解码与绘制调用。
// The reading-background picker panel was removed when its entry moved to Settings on
// 2026-10-25; this file keeps only the background decode and paint calls.
// 字体名称复用已加载的阅读字库；不让UI字体选择规则把它换成内建界面字形。
// Preview the name in the loaded reader face, bypassing the built-in UI-face preference.
static void draw_reading_font_name(uint8_t *fb,EpdRect box,const char *name) {
    char shown[TTF_FONT_NAME_MAX];copy_text(shown,sizeof(shown),name);
    int prior_slot=ttf_font_select(TTF_FONT_SLOT_SYSTEM);
    bool prior_aa=ttf_get_antialias();ttf_set_antialias(true);
    int px=ui_text_effective_px(27),above=0,below=0;
    while(px>1){
        ttf_measure_line_px(px,shown,&above,&below);
        if(above+below+2<=box.height)break;
        --px;
    }
    while(*shown && ttf_text_width_px(px,shown)>box.width){
        size_t n=strlen(shown)-1;
        while(n && ((unsigned char)shown[n]&0xc0)==0x80)--n;
        shown[n]=0;
    }
    ttf_measure_line_px(px,shown,&above,&below);
    ttf_draw_text_px(fb,box.x,box.y+box.height/2+(above-below)/2,px,shown,
                     EPD_DRAW_ALIGN_LEFT,UI_INK_BLACK,UI_INK_WHITE);
    ttf_set_antialias(prior_aa);ttf_font_select(prior_slot);
}
static void draw_font_pair(uint8_t *fb, EpdRect card, const char *title, const char *value) {
    ui_fill_round_rect(fb, card, 20, UI_GRAY_WHITE);
    ui_draw_control_frame(fb, card, 20, 0x70);
    ui_text_box(fb,(EpdRect){card.x+21,card.y+6,card.width-42,card.height/2-12},23,title,EPD_DRAW_ALIGN_LEFT,false);
    char shown[64]; copy_text(shown, sizeof(shown), value);
    EpdRect value_box={card.x+21,card.y+card.height/2,card.width-76,card.height/2-8};
    if (!strcmp(title,"阅读字体"))draw_reading_font_name(fb,value_box,shown);
    else ui_text_box(fb,value_box,27,shown,EPD_DRAW_ALIGN_LEFT,false);
    ui_draw_icon(fb,card.x+card.width-23,card.y+3*card.height/4,26,UI_ICON_CHEVRON_RIGHT,0x38);
}
static void reader_weight_value(char *out,size_t size,int steps){
    if(!steps){snprintf(out,size,"标准");return;}
    unsigned amount=steps<0?-steps:steps,whole=amount/16,fraction=(amount%16)*625;
    snprintf(out,size,"%c%u.%04u",steps<0?'-':'+',whole,fraction);
    size_t n=strlen(out);while(n&&out[n-1]=='0')out[--n]=0;if(n&&out[n-1]=='.')out[--n]=0;
}
static void draw_font_settings(uint8_t* fb) {
    // 界面标签独立于正文AA开关，退出后恢复正文设置。/ UI labels stay smooth independently of body AA; restore it afterward.
    bool prior_aa=ttf_get_antialias();ttf_set_antialias(true);
    draw_sheet(fb, reader_font_top(), "字体设置");
    int shown_px = s_reader_slider >= 0 ? s_reader_preview_px : s_px;
    char value[16]; snprintf(value, sizeof(value), "%d", shown_px);
    draw_pill_slider(fb, reader_slider_rect(0), "A", "A", value,
                     shown_px - BOOK_PX_MIN, BOOK_PX_MAX - BOOK_PX_MIN + 1, 20, 31);
    draw_font_pair(fb, s_font_card, "阅读字体", ttf_font_display_name());
    char weight_value[20];reader_weight_value(weight_value,sizeof(weight_value),app_settings_book_weight_fine());
    draw_font_pair(fb, s_weight_card, "字重", weight_value);
    draw_font_pair(fb, s_layout_card, "排版设置", "边距 · 行距");
    const bool book_fonts = app_settings_book_fonts();
    EpdRect card = s_bookfont_card;
    ui_fill_round_rect(fb, card, 20, UI_GRAY_WHITE); ui_draw_control_frame(fb, card, 20, 0x70);
    ui_text_box(fb,(EpdRect){card.x+21,card.y+6,card.width-42,card.height/2-12},23,"书内自带字体",EPD_DRAW_ALIGN_LEFT,false);
    ui_text_box(fb,(EpdRect){card.x+21,card.y+card.height/2,card.width-122,card.height/2-8},27,book_fonts?"开启":"关闭",EPD_DRAW_ALIGN_LEFT,false);
    EpdRect toggle = {card.x+card.width-89,card.y+3*card.height/4-19,69,39};
    ui_fill_round_rect(fb, toggle, 19, book_fonts ? 0x38 : 0xd0);
    int knob = toggle.x + (book_fonts ? 49 : 20);
    epd_fill_circle(knob, toggle.y + 19, 16, UI_GRAY_WHITE, fb);
    epd_draw_circle(knob, toggle.y + 19, 16, UI_GRAY_BLACK, fb);
    EpdRect rule = s_rule_card;
    ui_fill_round_rect(fb, rule, 20, UI_GRAY_WHITE); ui_draw_control_frame(fb, rule, 20, 0x70);
    static const char *const rule_names[] = {"无", "虚线", "点线"};
    int selected_rule = app_settings_book_reading_line();
    ui_text_box(fb,(EpdRect){rule.x+22,rule.y+10,142,rule.height-20},25,"阅读线",EPD_DRAW_ALIGN_LEFT,false);
    ui_text_box(fb,(EpdRect){rule.x+172,rule.y+10,77,rule.height-20},25,rule_names[selected_rule],EPD_DRAW_ALIGN_RIGHT,false);
    ui_draw_icon(fb,rule.x+rule.width-24,rule.y+rule.height/2,27,UI_ICON_CHEVRON_RIGHT,0x38);
    EpdRect aa = s_antialias_card;
    ui_fill_round_rect(fb, aa, 20, UI_GRAY_WHITE); ui_draw_control_frame(fb, aa, 20, 0x70);
    ui_text_box(fb,(EpdRect){aa.x+21,aa.y+10,aa.width-122,aa.height-20},25,"抗锯齿",EPD_DRAW_ALIGN_LEFT,false);
    draw_reading_switch(fb,(EpdRect){aa.x+aa.width-89,aa.y+aa.height/2-19,69,39},app_settings_book_antialias());
    ui_fill_round_rect(fb, (EpdRect){36, 1141, 612, 58}, 20, UI_GRAY_BLACK);
    reader_panel_text_vc(fb, 342, 1170, 28, "完成", EPD_DRAW_ALIGN_CENTER, true);
    ttf_set_antialias(prior_aa);
}

static EpdRect reader_weight_rect(int index) { return (EpdRect){index?542:56,909,86,86}; }
static int reader_weight_step(int x,int y) {
    if(ui_rect_hit(reader_weight_rect(0),x,y))return -1;
    if(ui_rect_hit(reader_weight_rect(1),x,y))return 1;
    return 0;
}
static void draw_weight_settings(uint8_t *fb) {
    draw_sheet(fb, 580, "字重");
    draw_sheet_back(fb, 580);
    reader_panel_text_vc(fb, 342, 730, 22, "调整阅读文字粗细", EPD_DRAW_ALIGN_CENTER, false);
    ttf_font_select(TTF_FONT_SLOT_SYSTEM);
    int weight=app_settings_book_weight_fine(),prior_weight=ttf_get_stroke_adjust_fine();
    ttf_set_stroke_adjust_fine(weight);
    ttf_draw_text_px(fb, 342, 833, 48, "阅读文字 Aa 123", EPD_DRAW_ALIGN_CENTER, 0, 15);
    ttf_set_stroke_adjust_fine(prior_weight);
    ui_fill_round_rect(fb,(EpdRect){36,895,612,114},57,0xe0);
    ui_draw_control_frame(fb,(EpdRect){36,895,612,114},57,0x30);
    for(int i=0;i<2;++i){EpdRect control=reader_weight_rect(i);epd_fill_circle(control.x+43,952,43,UI_GRAY_WHITE,fb);ui_draw_control_frame(fb,control,43,0x30);}
    ui_fill_rect_fast((EpdRect){87,951,24,3},weight<=-APP_BOOK_WEIGHT_FINE_MAX?UI_GRAY_LIGHT:UI_GRAY_BLACK,fb);
    ui_fill_rect_fast((EpdRect){573,951,24,3},weight>=APP_BOOK_WEIGHT_FINE_MAX?UI_GRAY_LIGHT:UI_GRAY_BLACK,fb);
    ui_fill_rect_fast((EpdRect){584,941,3,24},weight>=APP_BOOK_WEIGHT_FINE_MAX?UI_GRAY_LIGHT:UI_GRAY_BLACK,fb);
    char value[20];reader_weight_value(value,sizeof(value),weight);
    reader_panel_text_vc(fb,342,936,44,value,EPD_DRAW_ALIGN_CENTER,false);
    reader_panel_text_vc(fb,342,975,20,"像素 · 每次 1/16",EPD_DRAW_ALIGN_CENTER,false);
    reader_panel_text_vc(fb, 342, 1030, 20, "标准保留字体原始笔画", EPD_DRAW_ALIGN_CENTER, false);
    reader_panel_text_vc(fb, 342, 1070, 20, "同时适用于导入字体和书内字体", EPD_DRAW_ALIGN_CENTER, false);
    ui_fill_round_rect(fb, (EpdRect){36, 1141, 612, 58}, 20, UI_GRAY_BLACK);
    reader_panel_text_vc(fb, 342, 1170, 26, "完成", EPD_DRAW_ALIGN_CENTER, true);
}

static EpdRect rule_style_rect(int index) {
    return (EpdRect){36 + index * 208, 788, 196, 120};
}

static void draw_rule_settings(uint8_t *fb) {
    const int top = 580;
    draw_sheet(fb, top, "阅读线");
    draw_sheet_back(fb, top);
    reader_panel_text(fb, 42, 741, 21, "选择样式", EPD_DRAW_ALIGN_LEFT, false);
    static const char *const names[] = {"无", "虚线", "点线"};
    int selected = app_settings_book_reading_line();
    for (int i = 0; i < 3; ++i) {
        EpdRect card = rule_style_rect(i);
        ui_fill_round_rect(fb, card, 20, selected == i ? 0xc8 : 0xe8);
        ui_draw_control_frame(fb, card, 20, selected == i ? 0x40 : 0x78);
        reader_panel_text_vc(fb, card.x + card.width / 2, card.y + 37, 24, names[i], EPD_DRAW_ALIGN_CENTER, false);
        if (i) {
            int span = i == 1 ? 14 : 3, step = i == 1 ? 24 : 13;
            int start = card.x + 25, end = card.x + card.width - 25;
            for (int x = start; x < end; x += step)
                ui_fill_rect_fast((EpdRect){x, card.y + 83, x + span <= end ? span : end - x, 2}, UI_GRAY_BLACK, fb);
        } else reader_panel_text_vc(fb, card.x + card.width / 2, card.y + 83, 17, "不显示", EPD_DRAW_ALIGN_CENTER, false);
    }
    char offset[36];
    snprintf(offset, sizeof(offset), "上下位置  %+d", app_settings_book_reading_line_offset());
    reader_panel_text(fb, 42, 936, 21, offset, EPD_DRAW_ALIGN_LEFT, false);
    const EpdRect controls[] = {s_rule_offset_up, s_rule_offset_reset, s_rule_offset_down};
    const char *const labels[] = {"上移 2px", "恢复居中", "下移 2px"};
    for (int i = 0; i < 3; ++i) {
        ui_fill_round_rect(fb, controls[i], 20, selected ? 0xe0 : 0xf0);
        ui_draw_control_frame(fb, controls[i], 20, selected ? 0x60 : 0xa0);
        reader_panel_text_vc(fb, controls[i].x + controls[i].width / 2,
                   controls[i].y + controls[i].height / 2, 22, labels[i], EPD_DRAW_ALIGN_CENTER, false);
    }
    if (!selected) reader_panel_text(fb, 42, 1080, 18, "选择虚线或点线后可调整位置", EPD_DRAW_ALIGN_LEFT, false);
    EpdRect done = {36, 1113, 612, 72};
    ui_fill_round_rect(fb, done, 20, UI_GRAY_BLACK);
    reader_panel_text_vc(fb, 342, 1149, 23, "返回字体设置", EPD_DRAW_ALIGN_CENTER, true);
}

// 排版设置面板顶：为「标题样式」多让出一行，面板整体高了 80，原来的各行整体上移 80。
// Layout sheet top: one more row for the heading style means the sheet rises 80 px and every
// existing row moves up by the same 80.
#define READER_LAYOUT_SHEET_TOP 420

// 「缩进微调」在上、「标题样式」在下：两行同高，交换后仍各留 16 像素间隙。
// The indent nudge sits above the heading-style entry: both rows are the same height and keep a
// 16 px gap after the swap.
static EpdRect title_style_entry_rect(void) { return (EpdRect){36, 1052, 612, 66}; }

static EpdRect indent_choice_rect(int index) { return (EpdRect){214 + index * 108, 890, 100, 64}; }
static EpdRect indent_adjust_rect(int index) {
    const EpdRect controls[] = {{214,970,100,66}, {322,970,208,66}, {538,970,100,66}};
    return controls[index];
}
static EpdRect reader_layout_back_rect(void) { return (EpdRect){36,1132,612,72}; }
static int indent_adjust_hit(int x, int y) {
    for(int i=0;i<3;++i){
        EpdRect r=indent_adjust_rect(i);
        r.x-=4;r.width+=8;r.y-=6;r.height+=16;
        if(ui_rect_hit(r,x,y))return i;
    }
    return -1;
}
static void reader_indent_hold_cancel(void) {
    memset(&s_indent_hold,0,sizeof(s_indent_hold));
    s_indent_feedback_pending=false;
}
static void draw_indent_adjustments(uint8_t *fb) {
    bool preview=s_indent_hold.active&&!s_indent_hold.cancelled;
    int value=preview?s_indent_hold.value:app_settings_book_indent_adjust();
    char adjustment[28];snprintf(adjustment,sizeof(adjustment),"%+d px · 复位",value);
    const char *labels[]={"−",adjustment,"+"};
    for(int i=0;i<3;++i){
        EpdRect control=indent_adjust_rect(i);
        bool pressed=preview&&s_indent_hold.control==i;
        ui_fill_round_rect(fb,control,16,pressed?UI_GRAY_BLACK:0xf0);
        ui_draw_control_frame(fb,control,16,0x50);
        ui_text_box(fb,(EpdRect){control.x+8,control.y+5,control.width-16,control.height-10},i==1?22:31,labels[i],EPD_DRAW_ALIGN_CENTER,pressed);
    }
}
static app_redraw_t reader_indent_feedback(app_ctx_t *ctx) {
    ui_text_set_system_scale(true);draw_indent_adjustments(ctx->fb);ui_text_set_system_scale(false);
    s_area=(EpdRect){214,970,424,66};s_mode=MODE_DU;s_indent_feedback_pending=true;
    return APP_REDRAW_AREA;
}
static bool reader_indent_present(app_ctx_t *ctx,app_redraw_t redraw) {
    bool pending=s_indent_feedback_pending;s_indent_feedback_pending=false;
    if(!pending||redraw!=APP_REDRAW_AREA||s_view!=READING||s_reader_panel!=READER_PANEL_LAYOUT_SETTINGS||ctx->request_app)return false;
    guard_draw_result(ctx->hl,display_refresh_area(ctx->hl, DISPLAY_REFRESH_FEEDBACK, s_area, DISPLAY_REFRESH_DEFAULT));
    return true;
}
static void draw_layout_settings(uint8_t* fb) {
    const int top = READER_LAYOUT_SHEET_TOP;
    draw_sheet(fb, top, "排版设置");
    draw_sheet_back(fb, top);
    int shown_margin[MARGIN_COUNT];
    for (int i = 0; i < MARGIN_COUNT; ++i)
        shown_margin[i] = s_reader_slider >= 0 ? s_reader_preview_margin[i] : s_margin[i];
    int shown_line = s_reader_slider >= 0 ? s_reader_preview_line : app_settings_book_line_spacing();
    int shown_para = s_reader_slider >= 0 ? s_reader_preview_para : app_settings_book_paragraph_spacing();
    int shown_tracking = s_reader_slider >= 0 ? s_reader_preview_tracking : app_settings_book_tracking();
    // 四向边距排成 2×2：旋钮直接显示方向和像素，省掉每行外部标签。
    // The four margins form a 2x2 block: each knob carries its direction and pixel inset, so the
    // rows need no external labels.
    static const char* const margin_names[MARGIN_COUNT] = {"左", "右", "上", "下"};
    for (int i = 0; i < MARGIN_COUNT; ++i) {
        char text[16];
        snprintf(text, sizeof(text), "%s%d", margin_names[i], shown_margin[i]);
        draw_pill_slider(fb, reader_slider_rect(1 + i), "小", "大", text,
                         reader_margin_level(shown_margin[i]), BOOK_MARGIN_CHOICES, 21, 21);
    }
    int line_i = shown_line - 110;
    if (line_i < 0) line_i = 0;
    if (line_i > 40) line_i = 40;
    draw_pill_slider(fb, reader_slider_rect(5), "紧", "松", "行距", line_i, 41, 21, 21);
    char para_name[16]; snprintf(para_name, sizeof(para_name), "段%d%%", shown_para);
    draw_pill_slider(fb, reader_slider_rect(6), "紧", "松", para_name, app_settings_book_paragraph_level((uint8_t)shown_para), APP_BOOK_PARAGRAPH_COUNT, 21, 21);
    char track_name[24]; snprintf(track_name, sizeof(track_name), "字距%+d", shown_tracking + APP_BOOK_TRACK_MIN);
    draw_pill_slider(fb, reader_slider_rect(7), "紧", "松", track_name, shown_tracking, APP_BOOK_TRACK_COUNT, 21, 21);
    ui_text_box(fb,(EpdRect){42,890,154,64},24,"首行缩进",EPD_DRAW_ALIGN_LEFT,false);
    static const char* indent_names[] = {"无", "1字", "2字", "3字"};
    int indent = app_settings_book_indent();
    for (int i = 0; i < 4; ++i) {
        EpdRect choice = indent_choice_rect(i);
        bool selected = i == indent;
        ui_fill_round_rect(fb, choice, 16, selected ? UI_GRAY_BLACK : 0xe4);
        ui_draw_control_frame(fb, choice, 16, selected ? UI_GRAY_BLACK : 0x98);
        ui_text_box(fb,(EpdRect){choice.x+8,choice.y+5,choice.width-16,choice.height-10},23,indent_names[i],EPD_DRAW_ALIGN_CENTER,selected);
    }
    // 「缩进微调」在「标题样式」之上：标签与三段控件同一行，高度与下面的入口行一致。
    // The indent nudge sits above the heading-style entry: its label and trio share one row of the
    // same height as the entry row below.
    ui_text_box(fb,(EpdRect){42,970,154,66},24,"缩进微调",EPD_DRAW_ALIGN_LEFT,false);
    draw_indent_adjustments(fb);
    // 与字体页的设置卡片同款：白底细框，左标签、右当前值、右箭头。
    // Same card as the font sheet's settings rows: white fill, light frame, label left, value right.
    EpdRect style_row = title_style_entry_rect();
    ui_fill_round_rect(fb, style_row, 20, UI_GRAY_WHITE);
    ui_draw_control_frame(fb, style_row, 20, 0x70);
    ui_text_box(fb,(EpdRect){style_row.x+22,style_row.y+10,160,style_row.height-20},25,"标题样式",EPD_DRAW_ALIGN_LEFT,false);
    ui_text_box(fb,(EpdRect){style_row.x+182,style_row.y+10,style_row.width-234,style_row.height-20},25,
                reader_title_style_name(app_settings_book_title_style()),EPD_DRAW_ALIGN_RIGHT,false);
    ui_draw_icon(fb,style_row.x+style_row.width-24,style_row.y+style_row.height/2,27,UI_ICON_CHEVRON_RIGHT,0x38);
    EpdRect back = reader_layout_back_rect();
    ui_fill_round_rect(fb, back, 20, UI_GRAY_BLACK);
    reader_panel_text_vc(fb, 342, 1168, 24, "返回字体设置", EPD_DRAW_ALIGN_CENTER, true);
}

// 用户确认半屏双列布局；绘制与点击共用同一矩形，间隙不穿透。/ Approved two-column half sheet; paint and hit testing share rectangles with inert gaps.
static EpdRect reading_setting_rect(int index) {
    return ui_reader_settings_rect(app_settings_system_font_size(), index);
}
static int reading_settings_top(void) {
    return ui_reader_settings_top(app_settings_system_font_size());
}
static void draw_reading_switch(uint8_t *fb, EpdRect track, bool on) {
    ui_reader_settings_switch(fb, track, on);
}
static void draw_reading_settings(uint8_t* fb) {
    draw_sheet(fb, reading_settings_top(), "阅读设置");
    const ui_reader_settings_values_t values = {
        .system_font_size = app_settings_system_font_size(),
        .full_pages = app_settings_reader_full_pages(),
        .turn_effect = app_settings_reader_turn_effect(),
        .vertical_turn = app_settings_reader_vertical_turn(),
        .statusbar = app_settings_reader_statusbar(),
        .hide_images = app_settings_reader_hide_images(),
        .vertical_text = app_settings_book_vertical(),
    };
    ui_reader_settings_draw(fb, &values);
}

static void draw_turn_settings(uint8_t *fb) {
    draw_sheet(fb, 270, "翻页模式");
    draw_sheet_back(fb, 270);
    ui_reader_turn_settings_draw(fb, app_settings_reader_vertical_turn(), app_settings_reader_shake_turn());
}

static const char *reader_key_label(app_reader_key_action_t action) {
    static const char *const labels[APP_READER_KEY_COUNT] = {
        [APP_READER_KEY_PREV] = "上一页",
        [APP_READER_KEY_NEXT] = "下一页",
        [APP_READER_KEY_TOOLS] = "阅读工具栏",
        [APP_READER_KEY_FULLSCREEN] = "切换全屏",
        [APP_READER_KEY_HOME] = "返回主页",
        [APP_READER_KEY_REFRESH] = "手动全刷",
        [APP_READER_KEY_NONE] = "不操作",
        // 旧测试固件存过的 7 是已撤销动作，按不操作显示。/ Stored 7 is the withdrawn action and reads as no-op.
        [APP_READER_KEY_LEGACY_BACK] = "不操作",
        [APP_READER_KEY_FRONTLIGHT] = "前光开关",
    };
    return labels[action >= 0 && action < APP_READER_KEY_COUNT ? action : APP_READER_KEY_NONE];
}
// 面板只占下方三分之二；双击说明收在动作列表里，页面上不再重复。
// The sheet covers the lower two thirds; the double-press explanation lives in the picker,
// not on the sheet itself.
#define KEY_SETTINGS_TOP (UI_LOCK_HEIGHT / 3)
#define KEY_ICON_Y 545
#define KEY_LABEL_Y 600
#define KEY_CARDS_TOP 632
// 四行同高：中键长按、中键双击、上键单击、音量键翻页。视口正好三行，
// 滚动按整行对齐，卡片永远不会被切开。
// Four rows share one height: middle hold, middle double press, up-key single press and the
// volume switch. The viewport holds exactly three rows and scrolling snaps to a row, so no
// card is ever cut in half.
#define KEY_ROW_TOP 772
#define KEY_ROW_HEIGHT 96
#define KEY_ROW_GAP 12
#define KEY_ROW_PITCH (KEY_ROW_HEIGHT + KEY_ROW_GAP)
#define KEY_ROW_VISIBLE 3
#define KEY_ROW_VIEW_HEIGHT (KEY_ROW_VISIBLE * KEY_ROW_HEIGHT + (KEY_ROW_VISIBLE - 1) * KEY_ROW_GAP)
#define KEY_ROW_COUNT 4
#define KEY_ROW_MAX_SCROLL ((KEY_ROW_COUNT - KEY_ROW_VISIBLE) * KEY_ROW_PITCH)
// 行序号也决定拾取项：0 中键长按、1 中键双击、2 上键单击、3 音量键翻页。
// The row index doubles as the picker key: 0 hold, 1 double press, 2 up key, 3 volume switch.
static int s_key_scroll;
static bool s_key_drag;
static EpdRect reader_key_row_rect(int row) {
    return (EpdRect){36, KEY_ROW_TOP - s_key_scroll + row * KEY_ROW_PITCH, 612, KEY_ROW_HEIGHT};
}
// 本板盖板三键的物理顺序是中、左、右（中键在最左）；示意图按实体位置摆，
// 左侧圆圈才是中键。命中测试与绘制共用这张表，逻辑键号不变。
// This board's cover orders the keys middle, left, right (the middle key is leftmost), so the
// diagram follows the physical slots: the left circle is the middle key. Painting and hit
// testing share this table and the logical key indices stay unchanged.
static const unsigned s_reader_key_slots[3] = {1, 0, 2};
static EpdRect reader_key_card(unsigned slot) { return (EpdRect){36 + (int)slot * 208, KEY_CARDS_TOP, 196, 114}; }
// 视口外的整行不画；滚动量总是整行，所以不会出现半张卡片。
// Rows outside the viewport are skipped; the offset is always whole rows, so no card is partial.
static void draw_key_row(uint8_t *fb, int row) {
    static const char *const names[] = {"中键长按", "中键双击", "上键单击", "音量键翻页"};
    EpdRect card = reader_key_row_rect(row);
    if (card.y + card.height <= KEY_ROW_TOP || card.y >= KEY_ROW_TOP + KEY_ROW_VIEW_HEIGHT) return;
    ui_fill_round_rect(fb, card, 20, 0xf0);
    ui_draw_control_frame(fb, card, 20, 0x58);
    if (row == 3) {
        bool on = app_settings_reader_volume_turn();
        ui_text_box(fb,(EpdRect){58,card.y+24,300,48},26,names[row],EPD_DRAW_ALIGN_LEFT,false);
        ui_text_box(fb,(EpdRect){330,card.y+24,196,48},25,on?"开启":"关闭",EPD_DRAW_ALIGN_RIGHT,false);
        draw_reading_switch(fb,(EpdRect){556,card.y+28,69,39},on);
        return;
    }
    app_reader_key_action_t action = row == 0 ? app_settings_reader_hold_action()
        : row == 1 ? app_settings_reader_double_action() : app_settings_reader_up_action();
    ui_text_box(fb,(EpdRect){58,card.y+24,238,48},26,names[row],EPD_DRAW_ALIGN_LEFT,false);
    ui_text_box(fb,(EpdRect){322,card.y+24,270,48},25,reader_key_label(action),EPD_DRAW_ALIGN_RIGHT,false);
    ui_draw_icon(fb, 625, card.y + 48, 20, UI_ICON_CHEVRON_DOWN, 0x38);
}
static void draw_key_settings(uint8_t *fb) {
    draw_sheet(fb, KEY_SETTINGS_TOP, "按键控制");
    draw_sheet_back(fb, KEY_SETTINGS_TOP);
    static const char *const names[] = {"左键", "中键", "右键"};
    for (unsigned slot = 0; slot < 3; ++slot) {
        unsigned key = s_reader_key_slots[slot];
        int cx = 134 + (int)slot * 208;
        if (key == 1) {
            epd_draw_circle(cx, KEY_ICON_Y, 25, UI_GRAY_BLACK, fb);
            epd_draw_circle(cx, KEY_ICON_Y, 24, UI_GRAY_BLACK, fb);
        } else ui_fill_round_rect(fb, (EpdRect){cx - 26, KEY_ICON_Y - 3, 52, 6}, 3, UI_GRAY_BLACK);
        reader_panel_text_vc(fb, cx, KEY_LABEL_Y, 22, names[key], EPD_DRAW_ALIGN_CENTER, false);
        EpdRect card = reader_key_card(slot);
        ui_fill_round_rect(fb, card, 20, 0xf0);
        ui_draw_control_frame(fb, card, 20, 0x58);
        ui_text_box(fb,(EpdRect){card.x+17,card.y+7,card.width-57,36},20,"单击",EPD_DRAW_ALIGN_LEFT,false);
        ui_text_box(fb,(EpdRect){card.x+12,card.y+52,card.width-24,52},24,reader_key_label(app_settings_reader_key_action(key)),EPD_DRAW_ALIGN_CENTER,false);
        ui_draw_icon(fb, card.x + card.width - 23, card.y + 24, 20, UI_ICON_CHEVRON_DOWN, 0x38);
    }
    for (int row = 0; row < KEY_ROW_COUNT; ++row) draw_key_row(fb, row);
    reader_panel_text_vc(fb, 342, 1112, 21, "上下滑动查看更多", EPD_DRAW_ALIGN_CENTER, false);
    ui_fill_round_rect(fb, (EpdRect){36, 1141, 612, 58}, 20, UI_GRAY_BLACK);
    reader_panel_text_vc(fb, 342, 1170, 26, "完成", EPD_DRAW_ALIGN_CENTER, true);
}
// 动作列表的显示顺序；前光开关排在「不操作」前，存储值仍按枚举原值。
// Display order for the action list; the frontlight toggle sits before "no action" while
// stored values keep the enum's own numbering.
static const app_reader_key_action_t s_reader_key_order[] = {
    APP_READER_KEY_PREV, APP_READER_KEY_NEXT, APP_READER_KEY_TOOLS,
    APP_READER_KEY_FULLSCREEN, APP_READER_KEY_HOME, APP_READER_KEY_REFRESH,
    APP_READER_KEY_FRONTLIGHT, APP_READER_KEY_NONE,
};
#define READER_KEY_ACTION_ROWS ((int)(sizeof(s_reader_key_order) / sizeof(s_reader_key_order[0])))
#define KEY_ACTION_TOP 390
#define KEY_ACTION_PITCH 86
#define KEY_ACTION_HEIGHT 78
// 动作列表卡片：绘制与命中共用同一条矩形。八行时行距收到 86，底部提示行仍留在
// 「返回」按钮上方；此前绘制按 88 排、命中按 94，越靠下越容易点错。
// Action-list cards share one rectangle; eight rows tighten the pitch to 86 so the hint line
// still clears the Back button. Painting once used 88 while hit testing used 94, so lower rows
// fired on a different action than the one shown.
static EpdRect reader_key_action_card(int index) {
    return (EpdRect){36, KEY_ACTION_TOP + index * KEY_ACTION_PITCH, 612, KEY_ACTION_HEIGHT};
}
_Static_assert(KEY_ACTION_TOP + (READER_KEY_ACTION_ROWS - 1) * KEY_ACTION_PITCH + KEY_ACTION_HEIGHT < 1141,
               "key action rows must clear the footer button");
static void draw_key_picker(uint8_t *fb) {
    static const char *const titles[] = {"左键功能", "中键功能", "右键功能", "中键长按功能", "中键双击功能", "上键单击功能"};
    draw_sheet(fb, 270, titles[s_reader_key_pick < 6 ? s_reader_key_pick : 0]);
    draw_sheet_back(fb, 270);
    app_reader_key_action_t current = s_reader_key_pick == 3 ? app_settings_reader_hold_action()
        : s_reader_key_pick == 4 ? app_settings_reader_double_action()
        : s_reader_key_pick == 5 ? app_settings_reader_up_action()
        : app_settings_reader_key_action(s_reader_key_pick);
    for (int i = 0; i < READER_KEY_ACTION_ROWS; ++i) {
        app_reader_key_action_t action = s_reader_key_order[i];
        EpdRect card = reader_key_action_card(i);
        ui_fill_round_rect(fb, card, 20, current == action ? 0xd8 : UI_GRAY_WHITE);
        ui_draw_control_frame(fb, card, 20, 0x58);
        reader_panel_text_vc(fb, 58, card.y + 39, 27, reader_key_label(action), EPD_DRAW_ALIGN_LEFT, false);
        epd_draw_circle(608, card.y + 39, 11, UI_GRAY_BLACK, fb);
        if (current == action) epd_fill_circle(608, card.y + 39, 6, UI_GRAY_BLACK, fb);
    }
    reader_panel_text_vc(fb, 342, 1112, 20, s_reader_key_error[0] ? s_reader_key_error : "从底部上滑可随时打开阅读工具栏", EPD_DRAW_ALIGN_CENTER, false);
    ui_fill_round_rect(fb, (EpdRect){36, 1141, 612, 58}, 20, UI_GRAY_BLACK);
    reader_panel_text_vc(fb, 342, 1170, 26, "返回", EPD_DRAW_ALIGN_CENTER, true);
}

/* ---- 页眉页脚设置面板 / Header & footer chrome panels ---- */
// 六个槽位、两条高度加「恢复默认」整行滚动，标题下首屏显示全部六槽；选择页复用动作卡样式，超出视口两行。
// Six slots, two heights and a reset row scroll whole rows, with all six slots below the title; the picker reuses the
// action-card style and outgrows its viewport by two rows.
#define CHROME_ROW_TOP (KEY_SETTINGS_TOP + 104)
#define CHROME_ROW_HEIGHT 88
#define CHROME_ROW_GAP 8
#define CHROME_ROW_PITCH (CHROME_ROW_HEIGHT + CHROME_ROW_GAP)
#define CHROME_ROW_VISIBLE 6
#define CHROME_ROW_VIEW_HEIGHT (CHROME_ROW_VISIBLE * CHROME_ROW_HEIGHT + (CHROME_ROW_VISIBLE - 1) * CHROME_ROW_GAP)
#define CHROME_ROW_COUNT 9
#define CHROME_ROW_MAX_SCROLL ((CHROME_ROW_COUNT - CHROME_ROW_VISIBLE) * CHROME_ROW_PITCH)
#define CHROME_PICK_TOP 390
#define CHROME_PICK_PITCH 86
#define CHROME_PICK_HEIGHT 78
#define CHROME_PICK_VISIBLE 8
#define CHROME_PICK_VIEW_HEIGHT ((CHROME_PICK_VISIBLE - 1) * CHROME_PICK_PITCH + CHROME_PICK_HEIGHT)
// 显示顺序与存储枚举解耦：内容项在前，「不显示」压尾。/ Display order decouples from the stored
// enum: content items first, "blank" last.
static const app_reader_slot_item_t s_chrome_item_order[] = {
    APP_READER_SLOT_TITLE, APP_READER_SLOT_CHAPTER, APP_READER_SLOT_PAGE,
    APP_READER_SLOT_PAGE_PROGRESS, APP_READER_SLOT_PERCENT, APP_READER_SLOT_CLOCK,
    APP_READER_SLOT_BATTERY, APP_READER_SLOT_DATE, APP_READER_SLOT_ICON_BATTERY,
    APP_READER_SLOT_BLANK,
};
#define READER_CHROME_ITEM_ROWS ((int)(sizeof(s_chrome_item_order) / sizeof(s_chrome_item_order[0])))
#define CHROME_PICK_MAX_SCROLL ((READER_CHROME_ITEM_ROWS - CHROME_PICK_VISIBLE) * CHROME_PICK_PITCH)
_Static_assert(CHROME_ROW_TOP >= KEY_SETTINGS_TOP + 92, "chrome rows must clear the title");
_Static_assert(CHROME_ROW_TOP + CHROME_ROW_VIEW_HEIGHT <= 1090, "chrome rows must clear the hint");
_Static_assert(CHROME_PICK_TOP + CHROME_PICK_VIEW_HEIGHT < 1112, "chrome picker must clear the hint");
static EpdRect chrome_row_rect(int row) {
    return (EpdRect){36, CHROME_ROW_TOP - s_chrome_scroll + row * CHROME_ROW_PITCH, 612, CHROME_ROW_HEIGHT};
}
static bool chrome_row_visible(EpdRect card, int top, int height) {
    return card.y >= top && card.y + card.height <= top + height;
}
// 视口外的整行不画；滚动量总是整行，所以不会出现半张卡片。
// Rows outside the viewport are skipped; the offset is always whole rows, so no card is partial.
static void draw_chrome_row(uint8_t *fb, int row) {
    static const char *const names[] = {"页眉左侧", "页眉中间", "页眉右侧",
                                        "页脚左侧", "页脚中间", "页脚右侧"};
    EpdRect card = chrome_row_rect(row);
    if (!chrome_row_visible(card, CHROME_ROW_TOP, CHROME_ROW_VIEW_HEIGHT)) return;
    const int cy = card.y + card.height / 2;
    ui_fill_round_rect(fb, card, 20, 0xf0);
    ui_draw_control_frame(fb, card, 20, 0x58);
    if (row == CHROME_ROW_COUNT - 1) {
        reader_panel_text_vc(fb, 342, cy, 26, "恢复默认布局", EPD_DRAW_ALIGN_CENTER, false);
        return;
    }
    if (row >= 6) {
        const unsigned bar = (unsigned)row - 6;
        char value[24];
        snprintf(value, sizeof(value), "%d px", s_chrome_height[bar]);
        ui_text_box(fb, (EpdRect){58, cy - 24, 238, 48}, 26,
                    bar ? "页脚高度" : "页眉高度", EPD_DRAW_ALIGN_LEFT, false);
        ui_text_box(fb, (EpdRect){414, cy - 24, 140, 48}, 25, value, EPD_DRAW_ALIGN_CENTER, false);
        ui_text_box(fb, (EpdRect){338, cy - 32, 72, 64}, 32, "−", EPD_DRAW_ALIGN_CENTER, false);
        ui_text_box(fb, (EpdRect){566, cy - 32, 72, 64}, 32, "+", EPD_DRAW_ALIGN_CENTER, false);
        return;
    }
    const app_reader_slot_item_t item = app_settings_reader_chrome_item((unsigned)row / 3, (unsigned)row % 3);
    ui_text_box(fb,(EpdRect){58,cy-24,238,48},26,names[row],EPD_DRAW_ALIGN_LEFT,false);
    ui_text_box(fb,(EpdRect){322,cy-24,270,48},25,reader_chrome_item_label(item),EPD_DRAW_ALIGN_RIGHT,false);
    ui_draw_icon(fb, 625, cy, 20, UI_ICON_CHEVRON_DOWN, 0x38);
}
static void draw_chrome_settings(uint8_t *fb) {
    draw_sheet(fb, KEY_SETTINGS_TOP, "页眉页脚");
    draw_sheet_back(fb, KEY_SETTINGS_TOP);
    for (int row = 0; row < CHROME_ROW_COUNT; ++row) draw_chrome_row(fb, row);
    reader_panel_text_vc(fb, 342, 1112, 21, s_chrome_error[0] ? s_chrome_error : "上下滑动查看更多", EPD_DRAW_ALIGN_CENTER, false);
    ui_fill_round_rect(fb, (EpdRect){36, 1141, 612, 58}, 20, UI_GRAY_BLACK);
    reader_panel_text_vc(fb, 342, 1170, 26, "完成", EPD_DRAW_ALIGN_CENTER, true);
}
static EpdRect chrome_pick_card(int index) {
    return (EpdRect){36, CHROME_PICK_TOP - s_chrome_pick_scroll + index * CHROME_PICK_PITCH, 612, CHROME_PICK_HEIGHT};
}
static void draw_chrome_picker(uint8_t *fb) {
    static const char *const titles[2][3] = {
        {"页眉左侧内容", "页眉中间内容", "页眉右侧内容"},
        {"页脚左侧内容", "页脚中间内容", "页脚右侧内容"},
    };
    draw_sheet(fb, 270, titles[s_chrome_pick_bar < 2 ? s_chrome_pick_bar : 0]
                                 [s_chrome_pick_slot < 3 ? s_chrome_pick_slot : 0]);
    draw_sheet_back(fb, 270);
    const app_reader_slot_item_t current = app_settings_reader_chrome_item(s_chrome_pick_bar, s_chrome_pick_slot);
    for (int i = 0; i < READER_CHROME_ITEM_ROWS; ++i) {
        const app_reader_slot_item_t item = s_chrome_item_order[i];
        EpdRect card = chrome_pick_card(i);
        if (!chrome_row_visible(card, CHROME_PICK_TOP, CHROME_PICK_VIEW_HEIGHT)) continue;
        ui_fill_round_rect(fb, card, 20, current == item ? 0xd8 : UI_GRAY_WHITE);
        ui_draw_control_frame(fb, card, 20, 0x58);
        reader_panel_text_vc(fb, 58, card.y + 39, 27, reader_chrome_item_label(item), EPD_DRAW_ALIGN_LEFT, false);
        epd_draw_circle(608, card.y + 39, 11, UI_GRAY_BLACK, fb);
        if (current == item) epd_fill_circle(608, card.y + 39, 6, UI_GRAY_BLACK, fb);
    }
    reader_panel_text_vc(fb, 342, 1112, 20, s_chrome_error[0] ? s_chrome_error : "选择该位置显示的内容", EPD_DRAW_ALIGN_CENTER, false);
    ui_fill_round_rect(fb, (EpdRect){36, 1141, 612, 58}, 20, UI_GRAY_BLACK);
    reader_panel_text_vc(fb, 342, 1170, 26, "返回", EPD_DRAW_ALIGN_CENTER, true);
}
// 两个面板共用整行滚动：视口内起手的纵向手势按一行步进，落点始终对齐行边界。
// Both panels share whole-row scrolling: a vertical gesture starting inside the viewport steps one
// row and always lands on a row boundary.
static app_redraw_t chrome_list_scroll(app_ctx_t *ctx, const ui_gesture_event_t *ev,
                                       int view_top, int view_height, int pitch, int max_scroll,
                                       int *scroll, bool *drag) {
    if (ev->y0 < view_top || ev->y0 >= view_top + view_height) return APP_REDRAW_NONE;
    if (*drag) {
        if (ev->type != UI_GESTURE_MOVE) *drag = false;
        return APP_REDRAW_NONE;
    }
    int dy = (int)ev->y - ev->y0, dx = (int)ev->x - ev->x0;
    int distance = dy < 0 ? -dy : dy;
    if (ev->type == UI_GESTURE_MOVE) {
        int across = dx < 0 ? -dx : dx;
        if (distance < 108 || distance <= across) return APP_REDRAW_NONE;
    }
    bool up = ev->type == UI_GESTURE_SWIPE_U || (ev->type == UI_GESTURE_MOVE && dy < 0);
    int next = *scroll + (up ? pitch : -pitch);
    if (next < 0) next = 0;
    if (next > max_scroll) next = max_scroll;
    if (ev->type == UI_GESTURE_MOVE) *drag = true;
    if (next == *scroll) return APP_REDRAW_NONE;
    *scroll = next;
    render(ctx, ctx->fb);
    s_area = (EpdRect){0, view_top, UI_LOCK_WIDTH, view_height};
    s_mode = MODE_GL16;
    return APP_REDRAW_AREA;
}

/* ---- 标题样式选择面板 / Heading style picker ---- */
// 卡片里贴的是真实题头带 [y+4, y+176] 的 1:1 结果：四种样式的内容都落在这 172 像素里
// （最上沿是边框索引的框顶 y+10，最下沿是静页余白的细线 y+175）。
// A card holds the real heading band's [y+4, y+176] window at 1:1: every style fits in those 172
// pixels, the frame's y+10 being the topmost art and the plain hairline at y+175 the lowest.
// 用户 2026-10-17：面板占三分之二屏，内容按 214 一槽整槽滚动；视口正好三槽，卡片永远整张可见，
// 不需要给 ui_kit 加裁剪。槽 0 是说明与字号行，槽 1..4 是四张样式卡。
// User 2026-10-17: the sheet covers two thirds of the screen and scrolls in 214 px slots; the
// viewport is exactly three slots, so every card stays whole and ui_kit needs no clipping. Slot 0
// holds the note and size row, slots 1..4 the four style cards.
#define TITLE_STYLE_SHEET_TOP 405
#define TITLE_STYLE_VIEW_TOP 501
#define TITLE_STYLE_VIEW_BOTTOM 1143
#define TITLE_STYLE_VIEW_HEIGHT 642
#define TITLE_STYLE_VIEW_SLOTS 3
#define TITLE_STYLE_CARD_PITCH 214
#define TITLE_STYLE_CARD_HEIGHT 206
#define TITLE_STYLE_WINDOW_TOP 30
#define TITLE_STYLE_WINDOW_HEIGHT 176
#define TITLE_STYLE_FOOTER_TOP 1150
// 预览窗贴的是题头块本身：块在带内居中，窗口偏移就是带高与块高之差的一半。
// The preview window holds the heading block itself: the block is centred in the band, so the
// window's offset is half the difference between the band and the block.
#define TITLE_STYLE_BAND_WINDOW_TOP 59
_Static_assert(TITLE_STYLE_BAND_WINDOW_TOP ==
                   (READER_HEADING_BAND_HEIGHT - READER_HEADING_BLOCK_HEIGHT) / 2,
               "preview window must keep showing the centred heading block");
_Static_assert(TITLE_STYLE_WINDOW_HEIGHT == READER_HEADING_BLOCK_HEIGHT,
               "preview window height must equal the heading block height");
_Static_assert(TITLE_STYLE_VIEW_TOP + TITLE_STYLE_VIEW_HEIGHT == TITLE_STYLE_VIEW_BOTTOM,
               "viewport bottom must follow from its top and height");
_Static_assert(TITLE_STYLE_VIEW_HEIGHT == TITLE_STYLE_VIEW_SLOTS * TITLE_STYLE_CARD_PITCH,
               "viewport must hold whole slots only");
_Static_assert(TITLE_STYLE_SHEET_TOP == UI_LOCK_HEIGHT / 3,
               "sheet covers two thirds of the screen");

// 滚动量只取槽高的整数倍，绘制与命中共用；拖动闩保证一次手势只走一步。
// The offset is always a whole number of slots and shared by painting and hit testing; the drag
// latch keeps one step per gesture.
static int s_title_style_scroll;
static bool s_title_style_drag;

// 槽顶含滚动量：负值表示该槽已滚到视口上方。
// Slot top including the scroll offset: a negative value means the slot has scrolled above the viewport.
static int title_style_slot_y(int slot) {
    return TITLE_STYLE_VIEW_TOP - s_title_style_scroll + slot * TITLE_STYLE_CARD_PITCH;
}

static const char* const s_title_style_names[] = {"中央章徽", "边框索引", "左锋题签", "静页余白"};
static const char* const s_title_style_details[] = {
    "胶囊式章节序号居中悬于章名上方",
    "完整细框搭配主题色粗边，结构清晰醒目",
    "左对齐标题，以主题色竖线强调",
    "极简居中标题，以细线收束留白",
};
static const char* reader_title_style_name(int style) {
    return style >= 0 && style < APP_BOOK_TITLE_STYLE_COUNT ? s_title_style_names[style] : "";
}
static EpdRect title_style_card_rect(int index) {
    return (EpdRect){36, title_style_slot_y(index + 1), 612, TITLE_STYLE_CARD_HEIGHT};
}
static EpdRect title_style_window_rect(int index) {
    EpdRect card = title_style_card_rect(index);
    return (EpdRect){card.x, card.y + TITLE_STYLE_WINDOW_TOP, card.width, TITLE_STYLE_WINDOW_HEIGHT};
}
// 卡片上方那行说明：没有横排题头的书先说清楚，其余时候显示选中样式的描述。
// The line above the cards: a book without a horizontal heading says so, otherwise the selected
// style's description.
static const char* title_style_note(void) {
    if (!book_has_chapter_headings()) return "当前书籍没有章节题头，样式对 EPUB / MOBI 生效";
    if (app_settings_book_vertical()) return "竖排沿用两列题头，样式对横排生效";
    return s_title_style_details[app_settings_book_title_style()];
}
// 预览文案优先用当前章的编号与章名，没有题头时退回样本。
// Previews prefer the current chapter's label and title and fall back to the sample.
static const char* title_style_preview_label(void) {
    return s_chapter_heading_label[0] ? s_chapter_heading_label : "第十二章";
}
static const char* title_style_preview_title(void) {
    return s_chapter_heading_title[0] ? s_chapter_heading_title : "远方来信";
}
// 字号行：左标签加「−／当前值 · 复位／+」三段，与「缩进微调」同款；绘制与命中共用同一组矩形。
// 行顶相对槽 0 计算，随滚动一起移动。
// The size row: a left label plus the -/value-reset/+ trio, the same shape as the indent nudge, with
// one set of rectangles shared by painting and hit testing. Its top is relative to slot 0 and moves
// with the scroll.
#define TITLE_STYLE_SIZE_ROW_TOP 76
static EpdRect heading_size_rect(int index) {
    const EpdRect controls[] = {{214, TITLE_STYLE_SIZE_ROW_TOP, 100, 64},
                                {322, TITLE_STYLE_SIZE_ROW_TOP, 208, 64},
                                {538, TITLE_STYLE_SIZE_ROW_TOP, 100, 64}};
    EpdRect rect = controls[index];
    rect.y += title_style_slot_y(0);
    return rect;
}

// 面板里只显示当前档位，改档立刻重画面板；预览固定基准档，所以这里的反馈就是数字与位置本身。
// The row only reports the current level and a tap repaints the panel; previews stay at the reference
// level, so this row's feedback is the number itself.
static void draw_heading_size_row(uint8_t *fb) {
    char value[32];
    snprintf(value, sizeof(value), "%d px · 复位", app_settings_book_title_size());
    const char *labels[] = {"−", value, "+"};
    ui_text_box(fb, (EpdRect){42, heading_size_rect(0).y, 154, 64}, 24, "标题字号",
                EPD_DRAW_ALIGN_LEFT, false);
    for (int i = 0; i < 3; ++i) {
        EpdRect control = heading_size_rect(i);
        ui_fill_round_rect(fb, control, 16, 0xf0);
        ui_draw_control_frame(fb, control, 16, 0x50);
        ui_text_box(fb, (EpdRect){control.x + 8, control.y + 5, control.width - 16, control.height - 10},
                    i == 1 ? 22 : 31, labels[i], EPD_DRAW_ALIGN_CENTER, false);
    }
}

// 点三段控件：两端进退一档、中间回到基准档，越界不动。
// A tap on the trio: the ends step one level, the middle returns to the reference level, and the
// ends stop at the legal range.
static app_redraw_t apply_heading_size(int control) {
    int next = app_settings_book_title_size();
    if (control == 1) next = APP_BOOK_TITLE_SIZE_DEFAULT;
    else if (control == 0) next -= APP_BOOK_TITLE_SIZE_STEP;
    else next += APP_BOOK_TITLE_SIZE_STEP;
    if (next < APP_BOOK_TITLE_SIZE_MIN) next = APP_BOOK_TITLE_SIZE_MIN;
    if (next > APP_BOOK_TITLE_SIZE_MAX) next = APP_BOOK_TITLE_SIZE_MAX;
    app_settings_set_book_title_size((uint8_t)next);
    return APP_REDRAW_PAGE;
}

static void draw_title_style_picker(uint8_t* fb) {
    draw_sheet(fb, TITLE_STYLE_SHEET_TOP, "标题样式");
    draw_sheet_back(fb, TITLE_STYLE_SHEET_TOP);
    // 只画完整落在视口里的槽；滚动步长与槽高同为 214，落点永远对齐，不会切出半张卡。
    // Paint only slots fully inside the viewport; the step equals the slot height, so a card is never
    // cut in half.
    int first = s_title_style_scroll / TITLE_STYLE_CARD_PITCH;
    int selected = app_settings_book_title_style();
    for (int slot = first; slot < first + TITLE_STYLE_VIEW_SLOTS && slot <= APP_BOOK_TITLE_STYLE_COUNT; ++slot) {
        if (slot == 0) {
            int y = title_style_slot_y(0);
            ui_text_box(fb,(EpdRect){36,y+12,612,30},19,title_style_note(),EPD_DRAW_ALIGN_CENTER,false);
            draw_heading_size_row(fb);
            char current[64];
            snprintf(current, sizeof(current), "当前样式 · %s", reader_title_style_name(selected));
            ui_text_box(fb,(EpdRect){42,y+160,360,30},19,current,EPD_DRAW_ALIGN_LEFT,false);
            ui_text_box(fb,(EpdRect){420,y+160,222,30},19,"上滑查看更多",EPD_DRAW_ALIGN_RIGHT,false);
            continue;
        }
        int i = slot - 1;
        EpdRect card = title_style_card_rect(i);
        bool active = i == selected;
        // 卡片底 → 预览窗底 → 内容 → 卡片边框：白色预览窗铺到卡片边缘，边框必须最后画，
        // 否则底边与左右两边会被它擦掉，卡片看着像开口的。
        // Card fill, window fill, content, then the card frame: the white window reaches the card's
        // edge, so the frame has to come last or the bottom and side borders are erased.
        ui_fill_round_rect(fb, card, 20, active ? 0xe6 : 0xf2);
        EpdRect window = title_style_window_rect(i);
        ui_fill_rect_fast(window, UI_GRAY_WHITE, fb);
        ui_hairline(fb, window.y, window.x, window.width, 0xb0);
        ui_text_box(fb,(EpdRect){card.x+16,card.y+2,card.width-60,28},21,s_title_style_names[i],EPD_DRAW_ALIGN_LEFT,false);
        if (active) epd_fill_circle(card.x+card.width-27,card.y+16,7,UI_GRAY_BLACK,fb);
        // 窗口里就是真题头：把题头带摆到窗口对应的位置，用同一支绘制函数画一次。预览固定画基准档，
        // 不跟随字号设置——挑样式时四种画法要能一一对比，字号在下面那行单独调。
        // The window holds the real heading: place the band where the window looks and paint it with
        // the same function the reader uses. Previews always draw the reference level and never
        // follow the size setting, so the four styles stay directly comparable while the size has
        // its own row below.
        draw_reader_title_style(fb, (EpdRect){window.x, window.y - TITLE_STYLE_BAND_WINDOW_TOP,
                                              window.width, READER_HEADING_BAND_HEIGHT},
                                i, APP_BOOK_TITLE_SIZE_DEFAULT, title_style_preview_label(),
                                title_style_preview_title());
        ui_draw_control_frame(fb, card, 20, active ? UI_GRAY_BLACK : 0x88);
        if (active) ui_draw_control_frame(fb,(EpdRect){card.x+1,card.y+1,card.width-2,card.height-2},19,UI_GRAY_BLACK);
    }
    // 滚动条：五槽内容对三槽视口，拇指按比例落位；只有两档位移，开销可忽略。
    // Scroll indicator: five slots of content against a three-slot viewport, thumb placed by ratio.
    const int track_top = TITLE_STYLE_VIEW_TOP + 8;
    const int track_height = TITLE_STYLE_VIEW_HEIGHT - 16;
    const int content_height = (APP_BOOK_TITLE_STYLE_COUNT + 1) * TITLE_STYLE_CARD_PITCH;
    const int max_scroll = content_height - TITLE_STYLE_VIEW_HEIGHT;
    int thumb_height = track_height * TITLE_STYLE_VIEW_HEIGHT / content_height;
    int thumb_top = track_top + (track_height - thumb_height) * s_title_style_scroll / max_scroll;
    ui_fill_round_rect(fb,(EpdRect){660,track_top,4,track_height},2,0xd8);
    ui_fill_round_rect(fb,(EpdRect){660,thumb_top,4,thumb_height},2,0x78);
    ui_fill_round_rect(fb,(EpdRect){36,TITLE_STYLE_FOOTER_TOP,612,58},20,UI_GRAY_BLACK);
    reader_panel_text_vc(fb,342,TITLE_STYLE_FOOTER_TOP+29,24,"返回字体设置",EPD_DRAW_ALIGN_CENTER,true);
}

static int reader_font_picker_top(void) { return app_settings_system_font_size()>=150?270:584; }
static EpdRect reader_font_pick_rect(int slot) {
    bool large=app_settings_system_font_size()>=150;
    int columns=large?2:3;
    return (EpdRect){36+(slot%columns)*(large?314:204),
                     reader_font_picker_top()+(large?150:131)+(slot/columns)*146,
                     large?298:180,126};
}
static void draw_font_picker(uint8_t* fb) {
    const int top = reader_font_picker_top();
    draw_sheet(fb, top, "字体");
    epd_draw_line(55, top + 49, 64, top + 58, UI_GRAY_BLACK, fb);
    epd_draw_line(64, top + 58, 73, top + 49, UI_GRAY_BLACK, fb);
    int count = ttf_font_count();
    int pages = count > 0 ? (count + BOOK_FONT_PAGE - 1) / BOOK_FONT_PAGE : 1;
    char current[96];
    snprintf(current, sizeof(current), "当前选择 · %s  %d/%d", ttf_font_display_name(), s_font_page + 1, pages);
    fit_text(current, 20, 540);
    ui_text_box(fb,(EpdRect){90,top+94,504,32},20,current,EPD_DRAW_ALIGN_CENTER,false);
    int start = s_font_page * BOOK_FONT_PAGE;
    char active[TTF_FONT_PATH_MAX]; copy_text(active, sizeof(active), ttf_font_path());
    bool active_builtin = ttf_font_is_builtin();
    for (int slot = 0; slot < BOOK_FONT_PAGE && start + slot < count; ++slot) {
        const ttf_font_item_t* item = ttf_font_item(start + slot);
        EpdRect card = reader_font_pick_rect(slot);
        bool selected = !strcmp(active, item->path);
        ui_fill_round_rect(fb, card, 18, selected ? 0xc2 : 0xe4);
        ui_draw_control_frame(fb, card, 18, selected ? 0x48 : 0x98);
        char label[TTF_FONT_NAME_MAX];
        copy_text(label, sizeof(label), font_friendly_name(item));
        if (ttf_font_open(item->path) == ESP_OK) {
            fit_text(label,25,card.width-32);
            draw_font_name_with_current_face(fb, card.x + card.width/2, card.y + 63, label,card.width-32);
        } else {
            fit_fixed_text(label, ui_text_effective_px(25), card.width - 32);
            ui_text_box(fb,ui_inset_rect(card,16),25,label,EPD_DRAW_ALIGN_CENTER,false);
        }
        if (selected) epd_fill_circle(card.x+card.width-22,card.y+22,7,UI_GRAY_BLACK,fb);
    }
    if (active_builtin) (void)ttf_font_open_builtin();
    else if (active[0]) (void)ttf_font_open(active);
    if (!count) reader_panel_text_vc(fb, 342, 780, 24, "所选字体目录中没有可用字体", EPD_DRAW_ALIGN_CENTER, false);
}

static void draw_reader_stats(uint8_t* fb) {
    const int top = 768;
    draw_sheet(fb, top, "阅读统计");
    char values[3][32];
    snprintf(values[0], sizeof(values[0]), "%u%%", percent(s_reader_session.position.page));
    snprintf(values[1], sizeof(values[1]), "%lu 分钟", (unsigned long)(s_session_read_ms / 60000));
    snprintf(values[2], sizeof(values[2]), "%lu 次", (unsigned long)s_session_turns);
    const char* labels[] = {"阅读进度", "本次阅读", "翻页"};
    for (int i = 0; i < 3; ++i) {
        int cx = 114 + i * 228;
        ui_text_box(fb,(EpdRect){cx-100,top+100,200,48},33,values[i],EPD_DRAW_ALIGN_CENTER,false);
        ui_text_box(fb,(EpdRect){cx-100,top+154,200,36},20,labels[i],EPD_DRAW_ALIGN_CENTER,false);
        if (i < 2) ui_fill_rect_fast((EpdRect){228 + i * 228, top + 99, 1, 74}, UI_GRAY_LIGHT, fb);
    }
    ui_text_box(fb,(EpdRect){54,top+192,576,42},21,"本书进度",EPD_DRAW_ALIGN_LEFT,false);
    EpdRect track = {54, top + 245, 576, 12};
    ui_fill_round_rect(fb, track, 6, 0xd0);
    EpdRect fill = track; fill.width = fill.width * (int)percent(s_reader_session.position.page) / 100;
    if (fill.width) ui_fill_round_rect(fb, fill, 6, 0x40);
    if (book_chapter_count() == 1) {
        char left[32], right[32];
        snprintf(left, sizeof(left), "第 %u 页", (unsigned)s_reader_session.position.page + 1);
        snprintf(right, sizeof(right), "共 %u 页", (unsigned)book_layout_page_count());
        reader_panel_text(fb, 54, top + 272, 19, left, EPD_DRAW_ALIGN_LEFT, false);
        reader_panel_text(fb, 630, top + 272, 19, right, EPD_DRAW_ALIGN_RIGHT, false);
    } else {
        reader_panel_text(fb, 54, top + 272, 19, "当前页 —", EPD_DRAW_ALIGN_LEFT, false);
        reader_panel_text(fb, 630, top + 272, 19, "总页数 —", EPD_DRAW_ALIGN_RIGHT, false);
    }
    EpdRect details = {36, top + 319, 294, 78}, recent = {354, top + 319, 294, 78};
    ui_fill_round_rect(fb, details, 20, 0xd8); ui_draw_control_frame(fb, details, 20, 0x68);
    ui_fill_round_rect(fb, recent, 20, 0xd8); ui_draw_control_frame(fb, recent, 20, 0x68);
    char marks[32]; snprintf(marks, sizeof(marks), "查看书签 · %u", (unsigned)bookmark_count());
    ui_text_box(fb,(EpdRect){48,top+331,270,54},24,marks,EPD_DRAW_ALIGN_CENTER,false);
    ui_text_box(fb,(EpdRect){366,top+331,270,54},24,"最近 30 天",EPD_DRAW_ALIGN_CENTER,false);
}

static void draw_reader_bookmarks(uint8_t* fb) {
    const int top = 560;
    draw_sheet(fb, top, "书签");
    epd_draw_line(55, top + 49, 64, top + 58, UI_GRAY_BLACK, fb);
    epd_draw_line(64, top + 58, 73, top + 49, UI_GRAY_BLACK, fb);
    reader_bookmarks_t marks = {0};
    bool valid = bookmark_load(&marks);
    int pages = bookmark_pages(valid ? marks.count : 0);
    if (s_bookmark_page >= pages) s_bookmark_page = pages - 1;
    if (valid && marks.count) {
        EpdRect manage = {544, top + 30, 104, 54};
        ui_fill_round_rect(fb, manage, 20, s_bookmark_edit ? 0xd8 : 0xe8);
        ui_draw_control_frame(fb, manage, 20, 0x78);
        reader_panel_text_vc(fb, 596, top + 57, 20, s_bookmark_edit ? "完成" : "管理", EPD_DRAW_ALIGN_CENTER, false);
    }
    if (!valid || !marks.count) {
        reader_panel_text_vc(fb, 342, top + 260, 26, "还没有保存书签", EPD_DRAW_ALIGN_CENTER, false);
        reader_panel_text_vc(fb, 342, top + 309, 18, "阅读时点按“书签”即可保存当前位置", EPD_DRAW_ALIGN_CENTER, false);
        return;
    }
    int first = s_bookmark_page * bookmark_rows();
    for (int row = 0; row < bookmark_rows() && first + row < marks.count; ++row) {
        int newest = (int)marks.count - 1 - (first + row);
        const reader_bookmark_entry_t* mark = &marks.entries[newest];
        EpdRect item = bookmark_row_rect(row);
        bool current = mark->chapter == s_reader_session.position.chapter && mark->byte_off == book_layout_page_start_offset(s_reader_session.position.page);
        bool selected = s_bookmark_edit && (s_bookmark_selected & (UINT32_C(1) << newest));
        ui_fill_round_rect(fb, item, 14, selected ? 0xc0 : current ? 0xc8 : 0xe8);
        ui_draw_control_frame(fb, item, 14, current ? 0x50 : 0x98);
        char chapter[96];
        if (book_chapter_title(mark->chapter, chapter, sizeof(chapter)) != ESP_OK)
            snprintf(chapter, sizeof(chapter), "第 %u 节", (unsigned)mark->chapter + 1);
        char where[52];
        if (mark->chapter == s_reader_session.position.chapter)
            snprintf(where, sizeof(where), "第 %u 页", (unsigned)book_layout_page_for_offset(mark->byte_off) + 1);
        else snprintf(where, sizeof(where), "章节 %u", (unsigned)mark->chapter + 1);
        int end=item.x+item.width-(s_bookmark_edit?70:20);
        int where_width=ui_text_fixed_width_px(ui_text_effective_px(20),where);
        if(where_width>220)where_width=220;
        int chapter_width=end-where_width-item.x-34;
        fit_text(chapter,24,chapter_width);
        ui_text_box(fb,(EpdRect){item.x+18,item.y+5,chapter_width,item.height/2-10},24,chapter,EPD_DRAW_ALIGN_LEFT,false);
        ui_text_box(fb,(EpdRect){end-where_width,item.y+5,where_width,item.height/2-10},20,where,EPD_DRAW_ALIGN_RIGHT,false);
        ui_text_box(fb,(EpdRect){item.x+18,item.y+item.height/2,item.width-90,item.height/2-5},18,
                    s_bookmark_edit?"点按选择":current?"当前阅读位置":"点按跳转到此处",EPD_DRAW_ALIGN_LEFT,false);
        if (s_bookmark_edit) {
            epd_draw_circle(item.x + item.width - 35, item.y + 36, 12, UI_GRAY_BLACK, fb);
            if (selected) epd_fill_circle(item.x + item.width - 35, item.y + 36, 7, UI_GRAY_BLACK, fb);
        }
    }
    if (s_bookmark_edit) {
        EpdRect all = {36, top + 520, 258, 64}, remove = {312, top + 520, 336, 64};
        ui_draw_button(fb, all, "全选本页", false);
        char label[40]; snprintf(label, sizeof(label), "删除已选 · %u", bookmark_selected_count());
        ui_draw_button(fb, remove, label, bookmark_selected_count() > 0);
    }
    char page[32]; snprintf(page, sizeof(page), "‹   %d / %d   ›", s_bookmark_page + 1, pages);
    reader_panel_text_vc(fb, 342, top + 606, 22, page, EPD_DRAW_ALIGN_CENTER, false);
    if (s_bookmark_delete_confirm) {
        EpdRect panel = {54, top + 196, 576, 240};
        ui_fill_round_rect(fb, panel, 24, UI_GRAY_WHITE);
        ui_draw_control_frame(fb, panel, 24, 0x48);
        reader_panel_text_vc(fb, 342, top + 252, 27,
                   s_bookmark_delete_error ? "删除失败，请重试" : "删除选中的书签？", EPD_DRAW_ALIGN_CENTER, false);
        reader_panel_text_vc(fb, 342, top + 298, 18, "只删除书签，不影响阅读进度", EPD_DRAW_ALIGN_CENTER, false);
        ui_draw_button(fb, (EpdRect){76, top + 348, 244, 64}, "取消", false);
        ui_draw_button(fb, (EpdRect){364, top + 348, 244, 64}, "确认删除", true);
    }
}

static void draw_reader_stats_recent(uint8_t* fb) {
    const int top = 584;
    draw_sheet(fb, top, "最近 30 天");
    epd_draw_line(55, top + 49, 64, top + 58, UI_GRAY_BLACK, fb);
    epd_draw_line(64, top + 58, 73, top + 49, UI_GRAY_BLACK, fb);
    if (!s_recent_days_valid) {
        reader_panel_text_vc(fb, 342, top + 280, 23, "请先完成日期与时间设置", EPD_DRAW_ALIGN_CENTER, false);
        reader_panel_text_vc(fb, 342, top + 326, 18, "完成对时后开始记录每天的阅读时长", EPD_DRAW_ALIGN_CENTER, false);
        return;
    }
    uint64_t sum = 0;
    uint32_t maximum = 0;
    for (int i = 0; i < 30; ++i) {
        sum += s_recent_days[i];
        if (s_recent_days[i] > maximum) maximum = s_recent_days[i];
    }
    char total[64];
    snprintf(total, sizeof(total), "累计 %llu 小时 %llu 分钟",
             (unsigned long long)(sum / 3600), (unsigned long long)((sum / 60) % 60));
    reader_panel_text_vc(fb, 342, top + 112, 28, total, EPD_DRAW_ALIGN_CENTER, false);
    const int base_y = top + 438, max_h = 250;
    ui_hairline(fb, base_y, 42, 600, 0xb0);
    for (int i = 0; i < 30; ++i) {
        int h = maximum ? (int)((uint64_t)s_recent_days[i] * max_h / maximum) : 0;
        if (s_recent_days[i] && h < 5) h = 5;
        if (h) ui_fill_round_rect(fb, (EpdRect){47 + i * 20, base_y - h, 10, h}, 5, 0x38);
    }
    reader_panel_text(fb, 42, base_y + 22, 19, "30 天前", EPD_DRAW_ALIGN_LEFT, false);
    reader_panel_text(fb, 642, base_y + 22, 19, "今天", EPD_DRAW_ALIGN_RIGHT, false);
}

/* ---- 脚注 / Footnotes ---- */

// 链接里的 %XX 只在文件名和锚点上出现；解不开就当作普通字节。
// %XX appears only in file names and fragments; undecodable bytes pass through unchanged.
static bool note_decode(const char *src, size_t len, char *dst, size_t cap, size_t *out) {
    size_t n = 0;
    for (size_t i = 0; i < len; ++i) {
        unsigned char c = (unsigned char)src[i];
        // XHTML 里文件名中的 & 必须写成 &amp;；比较前还原，否则带 & 的文件名永远对不上。
        // XHTML requires &amp; for an ampersand in a file name; restore it before comparing or
        // such a name never matches.
        if (c == '&' && i + 5 <= len && !memcmp(src + i, "&amp;", 5)) { c = '&'; i += 4; }
        else if (c == '%' && i + 2 < len) {
            int hi = src[i + 1] >= '0' && src[i + 1] <= '9' ? src[i + 1] - '0' :
                     src[i + 1] >= 'a' && src[i + 1] <= 'f' ? src[i + 1] - 'a' + 10 :
                     src[i + 1] >= 'A' && src[i + 1] <= 'F' ? src[i + 1] - 'A' + 10 : -1;
            int lo = src[i + 2] >= '0' && src[i + 2] <= '9' ? src[i + 2] - '0' :
                     src[i + 2] >= 'a' && src[i + 2] <= 'f' ? src[i + 2] - 'a' + 10 :
                     src[i + 2] >= 'A' && src[i + 2] <= 'F' ? src[i + 2] - 'A' + 10 : -1;
            if (hi >= 0 && lo >= 0) { c = (unsigned char)((hi << 4) | lo); i += 2; }
        }
        if (n + 1 >= cap) return false;
        dst[n++] = (char)c;
    }
    dst[n] = 0;
    if (out) *out = n;
    return true;
}

static const blk_t *note_block_at(size_t off) {
    if (!s_blocks || !s_block_count) return NULL;
    size_t lo = 0, hi = s_block_count;
    while (lo + 1 < hi) {
        size_t mid = lo + (hi - lo) / 2;
        if (s_blocks[mid].offset <= off) lo = mid;
        else hi = mid;
    }
    return &s_blocks[lo];
}

// 只有 "[1]" 这样的标记才当脚注：目录链接、正文超链接仍按原翻页规则处理。
// Only markers like "[1]" count as footnotes; TOC entries and body hyperlinks keep the usual
// page-turn rules.
static bool note_is_marker(const char *text, size_t len) {
    size_t i = 0;
    while (i < len && (unsigned char)text[i] <= ' ') ++i;
    if (i >= len || text[i] != '[') return false;
    size_t digits = 0;
    ++i;
    while (i < len && text[i] >= '0' && text[i] <= '9' && digits < 4) { ++i; ++digits; }
    return digits && i < len && text[i] == ']';
}

// 点中的内链若指向本章锚点，就把锚点所在段落收进弹层。跨章链接暂不处理：
// 阅读页没有返回栈，跳过去会把当前位置弄丢。
// A tapped link that resolves to an anchor in this chapter puts that paragraph on the sheet.
// Cross-chapter targets are left alone: the reader has no back stack, so jumping would lose
// the current position.
static bool note_open_link(int link) {
    if (link < 0 || (size_t)link >= s_link_count || !s_links || !s_link_strings) return false;
    const html_link_t *entry = &s_links[link];
    if (entry->offset >= s_text_len || entry->len > s_text_len - entry->offset) return false;
    if (!note_is_marker(s_text + entry->offset, entry->len)) return false;
    const char *href = s_link_strings + entry->href;
    const char *hash = entry->href_len ? memchr(href, '#', entry->href_len) : NULL;
    if (!hash || hash + 1 >= href + entry->href_len) return false;
    size_t path_len = (size_t)(hash - href);
    if (path_len) {
        char chapter_name[128];
        if (book_chapter_source_name(s_reader_session.position.chapter, chapter_name, sizeof(chapter_name)) != ESP_OK)
            return false;
        char path[160];
        size_t path_bytes = 0;
        if (!note_decode(href, path_len, path, sizeof(path), &path_bytes)) return false;
        const char *base = strrchr(path, '/');
        base = base ? base + 1 : path;
        if (strcmp(base, chapter_name)) return false;
    }
    char fragment[128];
    size_t fragment_len = 0;
    if (!note_decode(hash + 1, entry->href_len - (size_t)(hash + 1 - href),
                     fragment, sizeof(fragment), &fragment_len) || !fragment_len)
        return false;
    size_t offset = SIZE_MAX;
    for (size_t i = 0; i < s_anchor_count; ++i) {
        const html_anchor_t *anchor = &s_anchors[i];
        if (anchor->id_len == fragment_len &&
            !memcmp(s_link_strings + anchor->id, fragment, fragment_len)) {
            offset = anchor->offset;
            break;
        }
    }
    if (offset == SIZE_MAX || offset >= s_text_len) return false;
    // 锚点通常落在注释段落的起点；落在段尾时让给下一段。
    // The anchor normally sits at the start of the note paragraph; at a block end, use the next one.
    const blk_t *block = note_block_at(offset);
    size_t start = offset, end = s_text_len;
    if (block) {
        // 段中间的锚点是脚注标记的返回点，不是注释段落；展开正文只会得到一个半段话。
        // An anchor in the middle of a block is the note's backlink target, not a note
        // paragraph; expanding it would show half a body paragraph.
        for (size_t i = block->offset; i < start && i < s_text_len; ++i)
            if ((unsigned char)s_text[i] > ' ') return false;
        size_t block_end = block->offset + block->len;
        if (start >= block_end && (size_t)(block - s_blocks) + 1 < s_block_count) {
            ++block;
            block_end = block->offset + block->len;
        }
        if (start < block->offset) start = block->offset;
        end = block_end;
    }
    if (start >= end) return false;
    while (start < end && ((unsigned char)s_text[start] <= ' ' || s_text[start] == '\r'))
        ++start;
    // 注释段以返回标记 "[1]" 开头；弹层里不需要它。
    // The note paragraph opens with the "[1]" backlink; the sheet does not need it.
    if (start < end && s_text[start] == '[') {
        size_t close = start + 1;
        while (close < end && close - start <= 8 && s_text[close] != ']') ++close;
        if (close < end && s_text[close] == ']') {
            start = close + 1;
            while (start < end && (unsigned char)s_text[start] <= ' ') ++start;
        }
    }
    while (end > start && (unsigned char)s_text[end - 1] <= ' ') --end;
    if (start >= end) return false;
    size_t len = end - start, limit = READER_NOTE_BYTES - 4;
    bool cut = len > limit;
    if (cut) {
        len = limit;
        // 截断只能落在整字边界上，否则最后会多出一个半个汉字。
        // Truncate on a codepoint boundary so the tail is not half a character.
        while (len && ((unsigned char)s_text[start + len] & 0xc0) == 0x80) --len;
    }
    memcpy(s_note_text, s_text + start, len);
    s_note_text[len] = 0;
    if (cut) memcpy(s_note_text + len, "…", sizeof("…"));
    s_reader_panel = READER_PANEL_NOTE;
    invalidate_prep();
    return true;
}

// 按 UI 字号折行；返回行数，truncated 表示还有内容没放下。
// Wrap by the UI font size; return the line count and whether content was left over.
static int note_wrap(int px, int width, int *starts, int *lens, int max_lines, bool *truncated) {
    static char line[1024];
    size_t total = strlen(s_note_text), pos = 0;
    int lines = 0;
    *truncated = false;
    while (pos < total && lines < max_lines) {
        size_t at = 0, best = 0;
        while (pos + at < total) {
            unsigned char lead = (unsigned char)s_note_text[pos + at];
            size_t n = lead < 0x80 ? 1 : lead >= 0xc2 && lead <= 0xdf ? 2 :
                       lead >= 0xe0 && lead <= 0xef ? 3 :
                       lead >= 0xf0 && lead <= 0xf4 ? 4 : 1;
            if (at + n >= sizeof(line)) break;
            memcpy(line, s_note_text + pos, at + n);
            line[at + n] = 0;
            if (ttf_text_width_px(px, line) > width) break;
            at += n;
            best = at;
        }
        // 单字就比整行还宽时至少前进一个完整字，不能切在 UTF-8 中间。
        // If one glyph is wider than the line, advance by that whole glyph and never split a
        // UTF-8 sequence.
        if (!best) {
            unsigned char lead = (unsigned char)s_note_text[pos];
            best = lead < 0x80 ? 1 : lead >= 0xc2 && lead <= 0xdf ? 2 :
                   lead >= 0xe0 && lead <= 0xef ? 3 :
                   lead >= 0xf0 && lead <= 0xf4 ? 4 : 1;
            if (best > total - pos) best = total - pos;
        }
        starts[lines] = (int)pos;
        lens[lines] = (int)best;
        ++lines;
        pos += best;
    }
    if (pos < total) *truncated = true;
    return lines;
}

static void draw_note_sheet(uint8_t *fb) {
    // 注释正文跟随阅读字体，但字号固定 32px：弹层是辅助信息，不随正文设置变大变小。
    // The note body uses the reader face at a fixed 32 px: the sheet is auxiliary information
    // and does not scale with the body setting.
    const int px = 32;
    int line_height = px * 150 / 100;
    const int width = UI_LOCK_WIDTH - 2 * 48;
    int available = UI_LOCK_HEIGHT - 150 - 104 - 56;
    int max_lines = available / line_height;
    if (max_lines > 18) max_lines = 18;
    if (max_lines < 1) max_lines = 1;
    int starts[18], lens[18];
    int previous_slot = ttf_font_selected();
    ttf_font_select(TTF_FONT_SLOT_SYSTEM);
    bool truncated = false;
    int lines = note_wrap(px, width, starts, lens, max_lines, &truncated);
    if (lines < 1) lines = 1;
    int top = UI_LOCK_HEIGHT - (104 + lines * line_height + 56);
    if (top < 150) top = 150;
    draw_sheet(fb, top, "注释");
    static char line[1024];
    for (int i = 0; i < lines; ++i) {
        size_t n = (size_t)lens[i];
        if (n >= sizeof(line)) n = sizeof(line) - 1;
        memcpy(line, s_note_text + starts[i], n);
        line[n] = 0;
        if (truncated && i == lines - 1) {
            size_t used = strlen(line);
            if (used + sizeof("…") <= sizeof(line)) memcpy(line + used, "…", sizeof("…"));
        }
        int baseline = top + 104 + i * line_height + ttf_ascender_px(px);
        ttf_draw_text_px(fb, 48, baseline, px, line, EPD_DRAW_ALIGN_LEFT, 0, 15);
    }
    reader_panel_text(fb, 342, top + 104 + lines * line_height + 4, 22,
                      "点击任意处关闭", EPD_DRAW_ALIGN_CENTER, false);
    ttf_font_select(previous_slot);
}

static void draw_reader_panel(uint8_t* fb) {
    if (s_reader_panel==READER_PANEL_NONE) return;
    ui_text_set_system_scale(true);
    if (s_reader_panel == READER_PANEL_TOOLS) {
        EpdRect band = reader_tools_area();
        ui_fill_rect_fast(band, UI_GRAY_WHITE, fb);
        // 1px 浅灰线在白底工具栏上几乎看不见；结构分隔改用深色双像素。
        // A 1 px light-gray rule reads as almost nothing on the white bar; use the dark two-pixel separator.
        ui_draw_separator(fb, band.y, 0, UI_LOCK_WIDTH, 0x70);
        const char* labels[] = {"目录", "书签", "阅读统计", "阅读设置", "字体设置",
                                s_reader_fullscreen ? "退出全屏" : "全屏"};
        for (int i = 0; i < BOOK_TOOL_COUNT; ++i) {
            EpdRect rect = tool_rect(i);
            int cx = rect.x + rect.width / 2;
            bool bookmarked = i == 1 && bookmark_is_current();
            draw_reader_tool_icon(fb, cx, 1138, i, bookmarked);
            ui_text_box(fb,(EpdRect){rect.x+6,1159,rect.width-12,40},20,bookmarked?"已书签":labels[i],EPD_DRAW_ALIGN_CENTER,false);
        }
    } else if (s_reader_panel == READER_PANEL_FONT_SETTINGS) draw_font_settings(fb);
    else if (s_reader_panel == READER_PANEL_WEIGHT_SETTINGS) draw_weight_settings(fb);
    else if (s_reader_panel == READER_PANEL_LAYOUT_SETTINGS) draw_layout_settings(fb);
    else if (s_reader_panel == READER_PANEL_RULE_SETTINGS) draw_rule_settings(fb);
    else if (s_reader_panel == READER_PANEL_FONT_PICKER) draw_font_picker(fb);
    else if (s_reader_panel == READER_PANEL_STATS) draw_reader_stats(fb);
    else if (s_reader_panel == READER_PANEL_BOOKMARKS) draw_reader_bookmarks(fb);
    else if (s_reader_panel == READER_PANEL_STATS_RECENT) draw_reader_stats_recent(fb);
    else if (s_reader_panel == READER_PANEL_REFRESH_SETTINGS) draw_reading_settings(fb);
    else if (s_reader_panel == READER_PANEL_TURN_SETTINGS) draw_turn_settings(fb);
    else if (s_reader_panel == READER_PANEL_KEY_SETTINGS) draw_key_settings(fb);
    else if (s_reader_panel == READER_PANEL_KEY_PICKER) draw_key_picker(fb);
    else if (s_reader_panel == READER_PANEL_CHROME_SETTINGS) draw_chrome_settings(fb);
    else if (s_reader_panel == READER_PANEL_CHROME_PICKER) draw_chrome_picker(fb);
    else if (s_reader_panel == READER_PANEL_TITLE_STYLE) draw_title_style_picker(fb);
    else if (s_reader_panel == READER_PANEL_NOTE) draw_note_sheet(fb);
    ui_text_set_system_scale(false);
}

/* ---- 绘制与预渲染 / Drawing and preparation ---- */
static uint8_t inline_ink_gray(uint8_t gray) {
    // 解码器已提供灰阶；再次加深并叠加系统对比度会压死抖动前的中间色阶。
    // The decoder already supplies grayscale; extra darkening and UI contrast clip midtones.
    return gray;
}

/* ---- 页眉页脚槽位数据 / Chrome slot data ---- */
// 槽位数据缓冲：章节名带「未保存 ·」前缀，时钟与日期来自状态栏同一来源，电量取自 PMU。
// Slot data buffers: the chapter name carries the unsaved prefix, clock and date share the
// status-bar source, and power comes from the PMU.
static char s_reader_chrome_chapter[96];
static char s_reader_chrome_clock[12];
static char s_reader_chrome_date[24];
static void reader_chrome_data(reader_chrome_data_t *data) {
    char chapter_name[72];
    reader_footer_chapter_name(chapter_name, sizeof(chapter_name));
    snprintf(s_reader_chrome_chapter, sizeof(s_reader_chrome_chapter), "%s%s",
             s_save_failed ? "未保存 · " : "", chapter_name);
    ui_nav_clock_text(s_reader_chrome_clock, sizeof(s_reader_chrome_clock));
    ui_nav_date_text(s_reader_chrome_date, sizeof(s_reader_chrome_date));
    const pmu_snapshot_t *pmu = device_power_get();
    *data = (reader_chrome_data_t){
        .title = s_reader_session.book.title[0] ? s_reader_session.book.title
                                                : s_reader_session.book.chapter_title,
        .chapter = s_reader_chrome_chapter,
        .page = (unsigned)s_reader_session.position.page,
        .page_count = (unsigned)book_layout_page_count(),
        .percent = percent(s_reader_session.position.page),
        .clock = s_reader_chrome_clock,
        .battery = pmu_battery_percent(pmu),
        .charging = pmu_battery_charging(pmu),
        .date = s_reader_chrome_date,
    };
}
// 翻页时对比页眉合成文本，决定顶部差分带是否随本帧推屏。
// Composed header texts decide on each turn whether the top diff band joins this frame.
static uint32_t reader_chrome_fingerprint(void) {
    reader_chrome_data_t data;
    reader_chrome_data(&data);
    return reader_chrome_header_fingerprint(&data);
}

static void draw_reader_header(uint8_t *fb) {
    // 信息居中，分隔线固定在信息栏下沿；默认高度保留原位置。
    // Center the slots and keep the rule at the bar's bottom; the default preserves original positions.
    const int top = app_settings_reader_statusbar() ? 80 : 0;
    reader_chrome_data_t data;
    reader_chrome_data(&data);
    reader_chrome_draw_header(fb, &data, top + s_chrome_height[0] / 2 + 1);
    ui_hairline(fb, top + s_chrome_height[0] - 2, 36, 612, UI_GRAY_LIGHT);
}
// 竖排章节题头：右列是编号（第三章），左列是章名，读序依旧自右向左；两列整体留出列宽，
// 排版用同一个宽度在首页右边缘预留，绘制与预留因此不会错位。
// A vertical chapter heading: the number column (第三章) on the right and the title on the left,
// still read right to left. Both columns reserve their width, and the layout reserves the same
// width at the right edge of the first page, so drawing and reservation cannot drift apart.
typedef struct {
    int width;       ///< 首页右起需要预留的列宽 / columns to reserve at the first page's right
    int title_px;    ///< 章名字号 / title type size
    int title_x;     ///< 章名列左边缘 / title column's left edge
    int label_px;    ///< 编号字号，无编号时为 0 / number size, 0 without a label
    int label_x;     ///< 编号列左边缘 / label column's left edge
    int text_height; ///< 较长一列的高度，用来垂直居中 / taller column, used to center
} reader_heading_t;

static reader_heading_t reader_heading_metrics(const char *title, const char *label, EpdRect body) {
    reader_heading_t metrics = {0};
    if (title == NULL || title[0] == 0 || body.height <= 0 || body.width <= 0) return metrics;
    // 题头跟横排的题头一样走系统字体：正文换卡上字体不改变章节标题。
    // The heading uses the system face exactly like the horizontal one, so swapping the body face
    // never changes chapter titles.
    ttf_font_select(TTF_FONT_SLOT_SYSTEM);
    const int avail = body.height - 48;
    int px = 48;
    int need = ttf_vertical_text_height_px(px, title, 0);
    // 一列放不下就缩字号；竖排的题头不折列，缩到 24 为止。
    // Shrink the size when one column cannot hold the title; a vertical heading does not break
    // into more columns and stops at 24.
    if (need > avail && avail > 0) {
        px = px * avail / need;
        px &= ~3;
        if (px < 24) px = 24;
    }
    int label_px = label != NULL && label[0] ? 24 : 0;
    const int gap = 12;
    metrics.title_px = px;
    metrics.label_px = label_px;
    metrics.width = px + (label_px ? label_px + gap : 0);
    // 题头最多占正文带的三分之一，留够列给正文。
    // A heading takes at most a third of the body band, leaving the rest for body columns.
    if (metrics.width > body.width / 3) {
        int scale = body.width / 3 * 100 / metrics.width;
        metrics.title_px = metrics.title_px * scale / 100;
        if (metrics.title_px < 24) metrics.title_px = 24;
        metrics.label_px = label_px ? label_px * scale / 100 : 0;
        if (metrics.label_px && metrics.label_px < 18) metrics.label_px = 18;
        metrics.width = metrics.title_px + (metrics.label_px ? metrics.label_px + gap : 0);
    }
    const int right = body.x + body.width;
    metrics.title_x = right - metrics.width;
    metrics.label_x = right - metrics.label_px;
    int title_h = ttf_vertical_text_height_px(metrics.title_px, title, 0);
    int label_h = metrics.label_px ? ttf_vertical_text_height_px(metrics.label_px, label, 0) : 0;
    metrics.text_height = title_h > label_h ? title_h : label_h;
    return metrics;
}

static void draw_vertical_heading(uint8_t *fb, EpdRect body) {
    reader_heading_t metrics = reader_heading_metrics(s_chapter_heading_title, s_chapter_heading_label, body);
    if (!metrics.width) return;
    int y = body.y + (body.height - metrics.text_height) / 2;
    if (y < body.y + 8) y = body.y + 8;
    ttf_font_select(TTF_FONT_SLOT_SYSTEM);
    if (metrics.label_px)
        ttf_draw_text_px_vertical(fb, metrics.label_x, y, metrics.label_px, s_chapter_heading_label, 0, 0, 15);
    ttf_draw_text_px_vertical(fb, metrics.title_x, y, metrics.title_px, s_chapter_heading_title, 0, 0, 15);
}

/* ---- 横排题头样式 / Horizontal heading styles ---- */
// 四种样式共用同一块题头带：带高即排版预留的高度，所以换样式与换字号都只重画首页题头，不重排整章。
// 字号按档缩放整块设计（48px 档就是历史那块整体移到带心），横向位置不随字号变，整块在带里居中。
// 任何档位、任何样式都必须留在带内；越界会压到正文上。
// All four styles share one band, and the band height is exactly what the layout reserves, so
// switching a style or a size repaints the first page instead of re-typesetting the chapter. The
// size scales the whole design (the 48 px level is the historical block moved to the band's centre)
// with horizontal positions fixed and the block centred in the band. Any level and any style has to
// stay inside the band, or it lands on the body text.
// 重排前把章节题头的几何重新说给排版：竖排留右侧列宽、横排留顶部行高，
// 绘制侧读的是同一组 s_chapter_lead_*。切模式、切全屏、改字号都必须走这里，
// 否则排版按旧模式预留、题头按新模式绘制，题头就会压到正文上。
// Tell the layout the chapter heading geometry before every re-layout: columns to the right in
// vertical setting, rows at the top in horizontal. Drawing reads the same s_chapter_lead_*
// values. Switching mode or full screen or changing the type size must all come through here;
// otherwise the layout reserves for the old mode while the heading draws in the new one and lands
// on top of the body.
// 题头几何按当前模式重算：竖排留右侧列宽、横排留顶部行高。
// height 由调用方按原有规矩算好（首块是插图、或没有可用标题时为 0），这里不再自己推导——
// 无条件按 214 算会把不该出现的题头带出来，正文里那份标题就与它并排成了两个标题。
// Heading geometry for the current mode: columns to the right vertically, rows at the top
// horizontally. `height` arrives already decided by the caller (zero when the first block is an
// illustration or no title is usable); deriving it here unconditionally brought a heading onto
// sheets that should have none, leaving it side by side with the body's own copy.
static void apply_chapter_lead(const char *title, const char *label, size_t skip, unsigned height) {
    s_chapter_lead_skip = skip;
    if (!book_has_chapter_headings() || title == NULL || title[0] == 0 || !height) {
        s_chapter_lead_height = s_chapter_lead_width = 0;
        book_layout_set_chapter_lead(0, 0);
        book_layout_set_chapter_lead_width(0);
        return;
    }
    if (app_settings_book_vertical()) {
        // 量竖排题头会光栅化字形，调用方须已持有绘制锁。
        // Measuring the vertical heading rasterizes glyphs; the caller must hold the draw lock.
        s_chapter_lead_height = 0;
        s_chapter_lead_width = (unsigned)reader_heading_metrics(title, label, body_rect()).width;
        book_layout_set_chapter_lead(skip, 0);
        book_layout_set_chapter_lead_width(s_chapter_lead_width);
    } else {
        s_chapter_lead_width = 0;
        s_chapter_lead_height = height;
        book_layout_set_chapter_lead(skip, s_chapter_lead_height);
        book_layout_set_chapter_lead_width(0);
    }
}

// 重排前把题头几何交给排版；调用方须持有绘制锁。所有正文重排都要走这一句，
// 否则换了模式或字号之后，排版按上一次的题头尺寸预留，题头就会压住正文。
// Hand the heading geometry to the layout before a re-layout; the caller holds the draw lock.
// Every body re-layout needs this line, otherwise a mode or size change reserves room for the
// previous heading and the heading lands on the body.
static void refresh_chapter_lead(void) {
    apply_chapter_lead(s_chapter_heading_title, s_chapter_heading_label, s_chapter_lead_skip,
                       s_chapter_lead_row_height);
}

// PR9 未知尺寸整页图没有混排槽；在调用侧适配原有位图缓存。/ PR9's unknown-size page has no inline slot; adapt the bitmap cache here.
static int reader_image_slots(size_t page) {
    int count = book_layout_page_image_count(page);
    return count ? count : book_layout_page_image(page) >= 0 ? 1 : 0;
}
static bool reader_image_slot(size_t page, int slot, int *index, int *y, int *width, int *height) {
    if (book_layout_page_image_count(page))
        return book_layout_page_image_at(page, slot, index, y, width, height);
    int image = book_layout_page_image(page);
    if (slot != 0 || image < 0) return false;
    // 未知尺寸的整页图同样通栏，不能拿正文栏宽当上限。
    // An unknown-size whole-page image bleeds as well, so the body column is not its ceiling.
    EpdRect band = book_layout_image_rect();
    if (index) *index = image;
    if (y) *y = 0;
    if (width) *width = band.width;
    if (height) *height = band.height;
    return true;
}
static void draw_reader_images(uint8_t *fb, size_t page, EpdRect body) {
    // 位图和布局代次必须匹配；预渲染另一页不能复用当前页插图。
    // Match page and layout generation; a prefetched page must never borrow current-page bitmaps.
    if (!app_settings_reader_hide_images() && page == s_reader_session.position.page &&
        s_page_images_for == page && s_page_images_generation == book_layout_generation()) {
        // 横向按插图区域居中：通栏时图片铺满整屏，不再留正文边距。
        // Center horizontally in the illustration region: with full bleed the image spans the
        // panel and no longer carries the body margins.
        EpdRect band = book_layout_image_rect();
        for (int i = 0; i < reader_image_slots(page); ++i) {
            int y, width, height;
            if (!reader_image_slot(page, i, NULL, &y, &width, &height)) continue;
            const reader_image_t *image = s_page_images && i < s_page_image_count ? &s_page_images[i] : NULL;
            if (image && image->gray) {
                int left = band.x + (band.width - image->width) / 2;
                int top = band.y + y + (height - image->height) / 2;
                // 页首插图贴到阅读区顶端：正文上方那点呼吸空间只属于正文，不该把插图压下来。
                // 用户设置的上边距则要保留，否则页首图会无视新留白。有阅读背景时插图跟着
                // 正文栏走，那道固定留白也要算进去，否则图会越过背景内框的上边。
                // A page-start illustration sits at the top of the reading area: the fixed inset
                // above body text belongs to text and must not push the illustration down. A
                // user-set top margin is honoured, otherwise the first image ignores it. With a
                // reading background the illustration follows the body column, so the fixed inset
                // counts too or the image crosses the background's top edge.
                if (y == 0 && !app_settings_book_background_path()[0])
                    top = reader_area().y + s_margin[MARGIN_TOP];

                for (int iy = 0; iy < image->height; ++iy)
                    for (int ix = 0; ix < image->width; ++ix)
                        epd_draw_pixel(left + ix, top + iy,
                            ui_image_dither_gray(inline_ink_gray(image->gray[(size_t)iy * image->width + ix]),
                                                 left + ix, top + iy), fb);
            } else if (height >= UI_PX_CAPTION + 4 && width >= 70) {
                char notice[64]; copy_text(notice, sizeof(notice), "此插图暂无法显示");
                fit_text(notice, UI_PX_CAPTION, width - 8);
                ui_text_vc(fb, body.x + body.width / 2, body.y + y + height / 2,
                           UI_PX_CAPTION, notice, EPD_DRAW_ALIGN_CENTER, false);
            }
        }
    }
}
// 页脚整行固定一种字体：有卡上字体时跟随阅读字体，否则用内建补充位图。
// 页脚字体面与量宽/绘制已随三段槽位迁入 main/book/reader_chrome.c。
// The footer face with its metrics and painting moved to main/book/reader_chrome.c with the slots.
// 只补画外框：进来的帧缓冲里已经有正文（整页重绘是 draw_reader 先清再画，预绘命中是整帧拷来的）。
// 这里再清一次会把刚画好的正文整块擦掉，页面上就只剩页眉、页脚和状态栏。
// Chrome only: the incoming frame already holds the body (draw_reader clears and paints it first;
// a prefetch hit copies the whole frame in). Clearing here again would erase the fresh body text and
// leave nothing on the page but the header, footer and status bar.
static void draw_reader_outer(uint8_t *fb,size_t page) {
    ui_text_set_system_scale(false);
    if (app_settings_reader_statusbar()) ui_nav_status(fb);
    if (!s_reader_fullscreen) draw_reader_header(fb);
    EpdRect body=body_rect();
    if (page == 0 && s_chapter_heading_title[0] &&
        (s_chapter_lead_height || s_chapter_lead_width)) {
        if (app_settings_book_vertical()) {
            // 竖排题头站在首页预留的那条列里，正文从它左边接着排。
            // The vertical heading stands in the columns the first page reserved; the body
            // continues to its left.
            draw_vertical_heading(fb, body);
        } else {
            // 样式与字号都来自阅读设置：只改这两个实参，绘制与排版预留仍然同源。
            // The style and the size both come from the reader's settings, so wiring them changes
            // these two arguments while painting stays tied to the reservation.
            draw_reader_title_style(fb, reader_heading_band(body.y),
                                    (int)app_settings_book_title_style(),
                                    (int)app_settings_book_title_size(),
                                    s_chapter_heading_label, s_chapter_heading_title);
        }
    }
    ui_reader_background_paint(fb, (EpdRect){0,0,UI_LOCK_WIDTH,UI_LOCK_HEIGHT}, true);
    draw_reader_images(fb, page, body);
    if (s_reader_fullscreen) {
        // Full-book progress uses the last few screen rows and no footer text.
        EpdRect track = {36, READER_FULLSCREEN_PROGRESS_TOP + 3, UI_LOCK_WIDTH - 72, 3};
        ui_fill_round_rect(fb, track, 1, 0x90);
        track.width = track.width * (int)percent(page) / 100;
        if (track.width) ui_fill_round_rect(fb, track, 1, UI_GRAY_BLACK);
    } else {
        EpdRect track = progress_rect();
        // 带内布局：条顶 track.y+8（距正文底 8px），文字行顶 track.y+27；条与文字 7px
        // 间隙，均落在 progress_rect 差分带内。
        // In-band layout: bar top at track.y+8 (8 px below the body), text top at track.y+27;
        // the 7 px bar-to-text gap stays inside the footer diff band.
        EpdRect bar = {track.x, track.y + 8, track.width, 12};
        ui_fill_round_rect(fb, bar, 5, UI_GRAY_LIGHT);
        bar.width = bar.width * (int)percent(page) / 100;
        if (bar.width) ui_fill_round_rect(fb, bar, 5, UI_GRAY_BLACK);
        // 页脚文字行三段槽位：字号不随正文设置，布局与截短在 reader_chrome 内完成。
        // The footer text row's three slots: the size never follows body settings; layout and
        // trimming live in reader_chrome.
        reader_chrome_data_t data;
        reader_chrome_data(&data);
        reader_chrome_draw_footer_row(fb, &data, track.y + 27, 28, track.x, track.x + track.width);
    }
    if (s_reader_notice[0]) {
        EpdRect notice = {218, 160, 248, 48};
        ui_fill_round_rect(fb, notice, 24, 0xd0);
        ui_text_vc(fb, 342, 184, 19, s_reader_notice, EPD_DRAW_ALIGN_CENTER, false);
    }
    draw_reader_panel(fb);
}
static void draw_reader(uint8_t *fb,size_t page) {
    ui_text_set_system_scale(false);ui_clear_page(fb);
    book_layout_draw_page(fb,page,body_rect(),s_px);
    draw_reader_outer(fb,page);
}
// 页面决定是否允许预绘；任务只借用本次推屏的稳定几何和布局。
// The page authorizes prefetch; the worker borrows stable geometry and layout for this presentation.
static bool kick_prep(void) {
    if (s_view != READING || !s_text || s_toolbar || s_clear_confirm ||
        s_reader_panel != READER_PANEL_NONE || s_reader_session.position.page + 1 >= book_layout_page_count() ||
        book_layout_page_image_count(s_reader_session.position.page) || book_layout_page_image_count(s_reader_session.position.page + 1)) return false;
    return reader_prefetch_start(s_reader_session.position.page + 1, body_rect(), s_px, fb_bytes());
}
static void reader_prefetch_idle(app_ctx_t *ctx) {
    if (s_view != READING || !s_text || s_toolbar || s_clear_confirm ||
        s_reader_panel != READER_PANEL_NONE || ctx->consumed || ctx->request_app ||
        ctx->request_menu || ctx->request_return || (ctx->touch && ctx->touch->touched)) return;
    if (book_layout_page_image_count(s_reader_session.position.page) || book_layout_page_image_count(s_reader_session.position.page + 1) ||
        s_reader_session.position.page + 1 >= book_layout_page_count()) return;
    reader_prefetch_work_idle(s_reader_session.position.page + 1);
}
static size_t current_toc_position(void) {
    size_t count = book_navigation_count();
    if (s_selected_toc < count && book_navigation_chapter(s_selected_toc) == s_reader_session.position.chapter)
        return s_selected_toc;
    size_t selected = SIZE_MAX;
    size_t best_chapter = 0;
    for (size_t i = 0; i < count; ++i) {
        size_t chapter = book_navigation_chapter(i);
        if (chapter > s_reader_session.position.chapter) break;
        if (selected == SIZE_MAX || chapter > best_chapter) { selected = i; best_chapter = chapter; }
    }
    return selected;
}
static void render(app_ctx_t* ctx, uint8_t* fb) {
#if CONFIG_READER_PERF
    READER_PERF_SCOPE(READER_PERF_PAINT);
#endif
    lock_draw();
    ui_text_set_system_scale(s_view != READING);
    if (s_view == CATEGORIES) { category_draw(fb); unlock_draw(); return; }
    if (s_view == EDIT) { draw_editor(fb); unlock_draw(); return; }
    if (s_view == SEARCH) { draw_search(fb); unlock_draw(); return; }
    if (s_view == IMPORT) { draw_import(fb); unlock_draw(); return; }
    if (s_view == BULK) { draw_bulk(fb, ctx->leaf); unlock_draw(); return; }
    if (s_view == MANAGE) {
        ui_clear_page(fb);
        ui_nav_status(fb);
        ui_nav_back(fb, 36, 79);
        ui_text_vc(fb, 342, 107, 34, "图书详情", EPD_DRAW_ALIGN_CENTER, false);
        ui_hairline(fb, 176, 36, 612, 0x70);
        draw_manage(fb);
        unlock_draw();
        return;
    }
    if (s_view == TOC) {
        book_toc_render(fb, s_reader_session.book.title[0] ? s_reader_session.book.title : s_reader_session.book.chapter_title,
                        book_navigation_count(), current_toc_position(), ctx->leaf, s_message,
                        reader_book_percent());
        if (s_toc_jump_open) {
            char target[160];
            reader_percent_label(s_toc_jump_percent, target, sizeof(target));
            book_toc_render_jump(fb, s_toc_jump_percent, target);
        }
        unlock_draw();
        return;
    }
    if (s_view == READING && s_text) {
        draw_reader(fb, s_reader_session.position.page);
        unlock_draw();
        return;
    }
    ui_clear_page(fb);
    // 留白保持纯白，灰阶只由封面和书架材质生成，避免整块背景被当成灰色驱动。
    // Keep whitespace white; only covers and shelf materials create grays, avoiding gray drive over the whole background.
    ui_fill_rect_fast((EpdRect){0,0,UI_LOCK_WIDTH,UI_NAV_TOP},UI_GRAY_WHITE,fb);
    // 题头和分页使用白底，避免整块灰底在进入书架时产生不必要的深色过渡。
    // Keep title and pagination white so entering the shelf does not drive an unnecessary gray-background transition.
    ui_fill_rect_fast((EpdRect){0,0,UI_LOCK_WIDTH,192},UI_GRAY_WHITE,fb);
    ui_fill_rect_fast((EpdRect){0,992,UI_LOCK_WIDTH,UI_NAV_TOP-992},UI_GRAY_WHITE,fb);
    ui_nav_status(fb);
    char category_title[BOOK_STORE_PATH_MAX];
    const int category_selected = book_shelf_categories_selected(&s_shelf_categories);
    if(s_shelf_selecting)snprintf(category_title,sizeof(category_title),"已选 %u 本",(unsigned)selected_count());
    else copy_text(category_title,sizeof(category_title),shelf_category_name(category_selected));
    EpdRect title=s_shelf_selecting?(EpdRect){36,80,256,80}:shelf_category_title_rect();
    if(!s_shelf_selecting)title.width-=54;
    int title_px=34;
    if(!s_shelf_selecting && category_selected<2)
        while(title_px>12 && ui_text_fixed_width_px(ui_text_effective_px(title_px),category_title)>title.width)--title_px;
    fit_text(category_title,title_px,title.width);
    shelf_label(fb,title,title_px,category_title,false);
    int title_width=ui_text_fixed_width_px(ui_text_effective_px(title_px),category_title);
    if(!s_shelf_selecting)ui_draw_icon(fb,36+title_width+27,120,26,
        book_shelf_categories_menu_is_open(&s_shelf_categories) ? UI_ICON_CHEVRON_UP : UI_ICON_CHEVRON_DOWN,UI_GRAY_BLACK);
    draw_shelf_header_button(fb, shelf_manage_rect(), 0);
    draw_shelf_header_button(fb, shelf_import_rect(), 1);
    if(!s_shelf_selecting)for(int action=2;action<4;++action)
        draw_shelf_header_button(fb,shelf_header_action_rect(action),action);
    if(s_shelf_selecting && category_selected>=2){
        EpdRect more=book_selection_rect(BOOK_SELECTION_MORE);
        ui_draw_control_frame(fb,more,8,selected_count()?0x50:0x90);
        ui_draw_icon(fb,more.x+more.width/2,more.y+more.height/2,34,UI_ICON_ELLIPSIS,selected_count()?0:0x90);
    }
    char shelf_summary[128];unsigned new_count=0;
    for(int n=0;n<s_count;++n)if(s_shelf[n].new_book&&!s_shelf[n].removed)++new_count;
    if(s_shelf_selecting)copy_text(shelf_summary,sizeof(shelf_summary),shelf_category_name(category_selected));
    else if(new_count)snprintf(shelf_summary,sizeof(shelf_summary),"新增 %u 本 · 归类",new_count);
    else shelf_summary[0]=0;
    if(s_shelf_selecting){
        EpdRect filter=book_selection_rect(BOOK_SELECTION_FILTER);
        ui_draw_control_frame(fb,filter,8,0x50);
        shelf_label(fb,(EpdRect){52,184,280,76},24,shelf_summary,false);
        ui_draw_icon(fb,360,222,26,UI_ICON_CHEVRON_DOWN,0);
        book_selection_button(fb,book_selection_rect(BOOK_SELECTION_PAGE_ALL),22,"全选本页",s_visible_count>0,false);
    } else if(shelf_summary[0])shelf_label(fb,(EpdRect){36,160,300,32},22,shelf_summary,false);
    if(!s_shelf_selecting && category_selected>=2){ui_draw_icon(fb,420,176,26,UI_ICON_PLUS,0);shelf_label(fb,(EpdRect){446,160,202,32},24,"添加图书",true);}
    if (app_settings_shelf_style() == 5) {
        EpdRect list=s_shelf_selecting?(EpdRect){36,280,612,720}:(EpdRect){36,184,612,788};
        ui_fill_round_rect(fb,list,20,UI_GRAY_WHITE);
        ui_draw_control_frame(fb,list,20,0x50);
    }
    for (int row = 0; row < shelf_rows(); ++row) {
        int i = ctx->leaf * shelf_rows() + row;
        if (i >= s_visible_count) break;
        EpdRect r = row_rect(row);
        char name[128]; copy_text(name, sizeof(name), s_shelf[i].name);
        // 封面抽出只改变书本位置，层板及触摸目标保持原位。
        // Lifting changes the book position only; shelf furniture and touch targets stay put.
        if (s_pressed_control == row)
            r.y -= app_settings_shelf_style() == 5 ? SHELF_LIST_BOUNCE_PX : SHELF_BOOK_LIFT_PX;
        draw_shelf_cover(fb, r, row, name, s_shelf[i].favorite);
        if (app_settings_shelf_style() == 5) draw_list_row(fb, row, &s_shelf[i]);
    }
    draw_shelf_furniture(fb);
    if (s_message[0] || s_shelf_warning[0])
        shelf_label(fb,(EpdRect){UI_MARGIN,s_shelf_selecting?998:984,612,22},17,s_message[0]?s_message:s_shelf_warning,false);
    if(app_settings_main_refresh_mode()==APP_MAIN_REFRESH_NORMAL){
        // 32列对齐后的封面/书架区；顶部状态栏、题头和下方分页不做黑色强整理。
        // Aligned cover/furniture area; status, title and lower pagination never enter the dark settling pass.
        const EpdRect artwork=s_shelf_selecting?(EpdRect){0,272,UI_LOCK_WIDTH,736}:(EpdRect){0,192,UI_LOCK_WIDTH,800};
        display_main_transition_gray_bands(&artwork,1);
    }
    draw_shelf_pager(fb, s_visible_count, ctx->leaf, leaves());
    if(s_shelf_selecting) {
        for(int row=0;row<shelf_rows() && ctx->leaf*shelf_rows()+row<s_visible_count;++row){
            EpdRect card=row_rect(row),check={card.x+12,card.y+10,38,38};
            ui_fill_round_rect(fb,check,5,UI_GRAY_WHITE);ui_draw_control_frame(fb,check,5,0);
            if(s_shelf[ctx->leaf*shelf_rows()+row].selected)ui_draw_icon(fb,check.x+19,check.y+19,32,UI_ICON_CHECK,0);
        }
        char selected[64];snprintf(selected,sizeof(selected),"查看已选 (%u)",selected_count());
        book_selection_button(fb,book_selection_rect(BOOK_SELECTION_REVIEW),26,selected,true,false);
        snprintf(selected,sizeof(selected),"归类 %u 本",selected_count());
        book_selection_button(fb,book_selection_rect(BOOK_SELECTION_SUBMIT),28,selected,selected_count()>0,true);
    } else if(s_category_result[0]) {
        shelf_label(fb,(EpdRect){36,1060,520,44},22,s_category_result,false);
        if(book_collection_can_undo(&s_collection))ui_draw_icon(fb,616,1082,32,UI_ICON_UNDO_2,0);
    }
    if(!s_shelf_selecting)ui_nav_draw(fb,1);
    if (book_shelf_categories_menu_is_open(&s_shelf_categories)) draw_category_menu(fb);
    unlock_draw();
}
static EpdRect reader_status_area(void) {
    // 状态栏和页眉合并到正文上沿，全屏只保留状态栏。
    // Merge status and header up to the body's top; full screen retains status alone.
    if (!s_reader_fullscreen)
        return (EpdRect){0, 0, UI_LOCK_WIDTH, (app_settings_reader_statusbar() ? 80 : 0) + s_chrome_height[0]};
    return (EpdRect){0, 0, UI_LOCK_WIDTH, 80};
}
static bool reader_status_visible(void) {
    // 窗口化页眉常驻时钟，全屏只有状态栏开档才有时间显示。
    // The windowed header always shows the clock; full screen only with the status bar on.
    return s_view==READING && s_text &&
        (app_settings_reader_statusbar() || !s_reader_fullscreen);
}
static void reader_status_paint(uint8_t *fb) {
    EpdRect area=reader_status_area();
    ui_fill_rect_fast(area,UI_GRAY_WHITE,fb);
    if (app_settings_reader_statusbar()) ui_nav_status(fb);
    if (!s_reader_fullscreen) draw_reader_header(fb);
    if (app_settings_book_background_path()[0])
        ui_reader_background_paint(fb,area,true);
}
// 阅读页单独记录已推屏电量，快捷栏的绘制不能代替页眉更新。
// Track presented reader power independently so quick-layer painting cannot acknowledge the header.
static int reader_status_power(void) {
    const pmu_snapshot_t *pmu = device_power_get();
    return (pmu_battery_percent(pmu) + 1) * 2 + (pmu_battery_charging(pmu) ? 1 : 0);
}
// 只在已有翻页请求中准备顶部，空闲分钟或电量变化不产生显示请求；页眉含页码/进度等
// 翻页相关槽位时，合成文本指纹随翻页变化，顶部带照样推屏。
// Prepare status only for an existing turn; idle minute/power changes never request output. When
// the header carries turn-dependent slots such as page or progress, the composed-text fingerprint
// changes with the turn and the top band still presents.
static bool reader_status_prepare(app_ctx_t *ctx, app_redraw_t redraw, int64_t minute, int power,
                                  uint32_t chrome) {
    if (redraw!=APP_REDRAW_AREA || !reader_status_visible() || s_reader_cleanup ||
        s_reader_panel!=READER_PANEL_NONE || s_toolbar || s_clear_confirm ||
        (minute==s_reader_clock_minute && power==s_reader_power &&
         s_reader_chrome_fp_valid && chrome==s_reader_chrome_fp)) return false;
    const EpdRect body=reader_area();
    if (s_area.x!=body.x || s_area.y!=body.y || s_area.width!=body.width || s_area.height!=body.height) return false;
    lock_draw(); reader_status_paint(ctx->fb); unlock_draw();
    return true;
}
// 正文是否是纯文字画面：插图页保留原来的整页差分，不让归一表的字芯推黑改动图片灰阶。
// 阅读背景不再排除：归一表把非黑相等对清成保持，整屏 pass 只重驱字芯，背景灰阶不动。
// Whether the body is a plain text frame: image pages keep the ordinary page differential
// so the normalize pass's black pushes cannot shift their picture grays. A reading
// background is no longer excluded: the normalize table holds every non-black equal pair,
// so the whole-screen pass re-drives only glyph cores and leaves background grays alone.
static bool reader_text_body(void) {
    return book_layout_page_image_count(s_reader_session.position.page) == 0;
}
#if CONFIG_READER_PERF && CONFIG_READER_MEMORY
static void reader_memory_snapshot(const char *event) {
    if (!reader_perf_memory_ready()) return;
    reader_memory_stats_t stats = {0};
    ttf_font_memory_stats(&stats);
    book_layout_memory_stats(&stats);
    reader_prefetch_memory_stats(&stats);
    ui_reader_background_memory_stats(&stats);
    const html_text_t *chapter = reader_session_content(&s_reader_session);
    stats.chapter = reader_memory_owned(chapter->utf8) + reader_memory_owned(chapter->blocks) +
        reader_memory_owned(chapter->images) + reader_memory_owned(chapter->runs) +
        reader_memory_owned(chapter->links) + reader_memory_owned(chapter->anchors) +
        reader_memory_owned(chapter->strings);
    for (size_t i = 0; i < chapter->image_count; ++i)
        stats.chapter += reader_memory_owned(chapter->images[i]);
    stats.page_images = reader_memory_owned(s_page_images);
    for (int i = 0; i < s_page_image_count; ++i)
        stats.page_images += reader_memory_owned(s_page_images[i].gray);
    stats.shelf_ui = reader_memory_owned(s_shelf) + reader_memory_owned(s_guard_cache) +
        reader_memory_owned(s_editor_cover);
    for (unsigned i = 0; i < BOOK_SHELF_COVER_SLOTS; ++i) {
        const book_shelf_cover_t *cover = book_shelf_covers_slot(&s_shelf_covers, i);
        stats.shelf_ui += reader_memory_owned(cover->gray) + reader_memory_owned(cover->fast_bits);
    }
    reader_perf_memory(event, &stats);
    reader_source_memory_stats_t source = {0};
    book_source_memory_stats(&source);
    reader_perf_source_memory(event, &source);
}
#endif

static bool present(app_ctx_t* ctx, app_redraw_t redraw) {
    // 开关标记只对本次推屏有效：提前返回也不留给下一帧。
    // The toggle flag applies to this present only; consume it even on an early return.
    const bool tools_toggle = s_reader_tools_toggle;
    s_reader_tools_toggle = false;
    bool tools_band = false;
    if(reader_indent_present(ctx,redraw))return true;
    if(s_shelf_key_paint_pending && redraw==APP_REDRAW_AREA && s_view==SHELF && !ctx->request_app){
        s_shelf_key_paint_pending=false;
        guard_draw_result(ctx->hl,display_refresh_area(ctx->hl, DISPLAY_REFRESH_FEEDBACK, s_area, DISPLAY_REFRESH_DEFAULT));
        return true;
    }
    // 输入不准备封面或正文；候选停顿只整理词栏，不累计整页清屏次数。
    // Input skips cover/body preparation; idle candidate settling affects only its strip and not whole-page cleanup counting.
    if (redraw == APP_REDRAW_AREA && (s_view == EDIT || s_view == SEARCH || s_view == CATEGORIES)) {
        if (s_input_settle) {
            guard_draw_result(ctx->hl, display_refresh_area(ctx->hl, DISPLAY_REFRESH_SETTLE, s_area, DISPLAY_REFRESH_DEFAULT));
            s_input_settle = false;
            s_du_count = 0;
            return true;
        }
        guard_draw_result(ctx->hl, display_refresh_area(ctx->hl, s_mode == MODE_DU ? DISPLAY_REFRESH_FEEDBACK : DISPLAY_REFRESH_DETAIL, s_area, DISPLAY_REFRESH_DEFAULT));
        s_du_count = 0;
        return true;
    }
    if (redraw == APP_REDRAW_NONE || redraw == APP_REDRAW_DONE || ctx->request_app) return true;
    if (s_presented_view != (int)s_view && redraw == APP_REDRAW_AREA) redraw = APP_REDRAW_PAGE;
    // 开书首帧走整屏 GC16：差分只驱动变化像素，首页深灰边线与文字会留下浅灰残影，
    // 全像素黑白循环才能把旧画面彻底擦掉。
    // Present the first frame after opening a book as a whole-screen GC16: a differential
    // drives changed pixels only and leaves the previous page's dark lines as gray ghosts;
    // a full-pixel black/white cycle erases the old frame.
    if (s_reader_entry_full) {
        s_reader_entry_full = false;
        if (s_view == READING) redraw = APP_REDRAW_FULL;
    }
    if ((s_view == SHELF || s_view == MANAGE) &&
        !(redraw == APP_REDRAW_AREA && (s_shelf_feedback_pending || s_cover_batch_present_pending))) {
        prepare_covers(ctx);
        gather_cover_entry(ctx);
    }
    if (ctx->request_app) return true;
    if (s_view == READING && s_text) {
        (void)ui_reader_background_prepare(app_settings_book_background_path());
        prepare_inline_image();
    }
    int64_t start = esp_timer_get_time();
    const int64_t frame_minute=ui_nav_clock_minute();
    const int frame_power=reader_status_power();
    const uint32_t frame_chrome=s_view == READING && s_text ? reader_chrome_fingerprint() : 0;
    if (redraw == APP_REDRAW_PAGE || redraw == APP_REDRAW_FULL) render(ctx, ctx->fb);
    const bool status_pending=reader_status_prepare(ctx,redraw,frame_minute,frame_power,frame_chrome);
    bool prep = kick_prep();
    int64_t drawn = esp_timer_get_time();
#if CONFIG_READER_PERF
    reader_perf_scope_t display_scope = reader_perf_scope_begin(READER_PERF_DISPLAY);
#endif
    // 本地细点阵挡板已是黑白目标，不注册灰区，省去暂存和第二轮GL16。
    // Local fine-dot guards are already BW targets; omit gray bands to skip staging and the second GL16 pass.
    if (s_view == SHELF && s_shelf_page_pending) display_main_transition_shelf_page();
    s_shelf_page_pending = false;
    enum EpdDrawError err;
    bool image_gray_refresh = s_reader_image_refresh_pending && s_view == READING &&
        s_reader_panel == READER_PANEL_NONE && !s_toolbar && !s_clear_confirm;
    const reader_present_request_t output_request = {
        .refresh = redraw == APP_REDRAW_AREA ? READER_PRESENT_AREA :
                   redraw == APP_REDRAW_FULL ? READER_PRESENT_FULL : READER_PRESENT_PAGE,
        .mode = s_mode, .page_mode = APP_PAGE_REFRESH_MODE,
        .area = s_area, .body_area = reader_area(), .status_area = reader_status_area(), .footer_area = progress_rect(),
        .progress_area = reader_fullscreen_progress_area(), .tools_area = reader_tools_area(),
        .screen_area = {0, 0, UI_LOCK_WIDTH, UI_LOCK_HEIGHT},
        .water_dir = s_water_turn_dir,
        .reading = s_view == READING, .reader_body = s_view == READING && s_text,
        .prior_reader = s_presented_view == (int)READING,
        .image_page = reader_image_slots(s_reader_session.position.page) > 0,
        .image_gray_refresh = image_gray_refresh,
        .water_effect = app_settings_reader_turn_effect() == 1,
        .cleanup = s_reader_cleanup, .tools_toggle = tools_toggle,
        .closing_tools_to_text = s_presented_panel == READER_PANEL_TOOLS &&
            s_reader_panel == READER_PANEL_NONE && reader_text_body(),
        .closing_panel_to_text = s_presented_panel != READER_PANEL_NONE &&
            s_reader_panel == READER_PANEL_NONE && reader_text_body(),
        .feedback = s_view == SHELF && s_shelf_feedback_pending,
        .fullscreen = s_reader_fullscreen, .turn_pending = s_reader_turn_pending,
        .water_turn_pending = s_water_turn_pending,
        .status_pending = status_pending, .footer_pending = s_reader_footer_pending,
        .footer_strong = s_reader_footer_strong,
        .hold_gray = normalize_hold_gray(ctx),
    };
    reader_present_result_t output = reader_present_output(ctx->hl, &output_request);
    err = output.error;
    tools_band = output.tools_band;
    if (output.footer_updated) {
        if (s_reader_footer_strong)
            ESP_LOGI(TAG, "reader footer gl16 pct=%u", percent(s_reader_session.position.page));
        else
            ESP_LOGI(TAG, "reader regions body_h=%d footer=diff pct=%u", s_area.height, percent(s_reader_session.position.page));
    }
    int64_t displayed = esp_timer_get_time();
#if CONFIG_READER_PERF
    reader_perf_scope_end(&display_scope);
    reader_perf_scope_t join_scope = reader_perf_scope_begin(READER_PERF_JOIN);
#endif
    reader_prefetch_end(prep);
#if CONFIG_READER_PERF
    reader_perf_scope_end(&join_scope);
#endif
    if (redraw == APP_REDRAW_AREA && s_mode == MODE_DU &&
        !s_reader_cleanup && !image_gray_refresh && !s_shelf_feedback_pending) {
        s_du_area = s_du_count ? ui_rect_union(s_du_area, s_area) : s_area;
        ++s_du_count;
        s_du_ms = esp_timer_get_time() / 1000;
    } else s_du_count = 0;
    ESP_LOGI(TAG, "present draw=%lld display=%lld join=%lld ms", (drawn - start) / 1000,
             (displayed - drawn) / 1000, (esp_timer_get_time() - displayed) / 1000);
    guard_draw_result(ctx->hl, err);
#if CONFIG_READER_PERF
    reader_perf_finish(err == EPD_DRAW_SUCCESS);
#endif
#if CONFIG_READER_PERF && CONFIG_READER_MEMORY
    reader_memory_snapshot("present");
#endif
    book_shelf_covers_ack(&s_shelf_covers,
        (s_view == SHELF || s_view == MANAGE) && err == EPD_DRAW_SUCCESS,
        (redraw == APP_REDRAW_PAGE || redraw == APP_REDRAW_FULL) && shelf_cover_frame_ready(),
        s_cover_batch_present_pending);
    book_shelf_covers_ack(&s_category_covers,
        s_view == CATEGORIES && s_category_ui.screen == CATEGORY_BOOKS && err == EPD_DRAW_SUCCESS,
        (redraw == APP_REDRAW_PAGE || redraw == APP_REDRAW_FULL) && !book_shelf_covers_pending(&s_category_covers), false);
    if (err==EPD_DRAW_SUCCESS && reader_status_visible() && !tools_band &&
        (redraw==APP_REDRAW_PAGE || redraw==APP_REDRAW_FULL || s_reader_cleanup || output.status_updated)) {
        s_reader_clock_minute=frame_minute;
        s_reader_power=frame_power;
        s_reader_chrome_fp=frame_chrome;
        s_reader_chrome_fp_valid=true;
    }
    const bool reader_frame_ok = err == EPD_DRAW_SUCCESS && s_view == READING && s_text &&
        s_reader_panel == READER_PANEL_NONE && !s_toolbar && !s_clear_confirm &&
        book_layout_page_image_count(s_reader_session.position.page) == 0;
    s_reader_text_frame = reader_frame_ok && !app_settings_book_background_path()[0];
    s_reader_stable_frame = reader_frame_ok;
    s_reader_cleanup = false;
    s_reader_image_refresh_pending = false;
    s_presented_view = (int)s_view;
    s_presented_panel = (int)s_reader_panel;
    s_reader_footer_pending = false;
    s_reader_footer_strong = false;
    s_water_turn_pending = false;
    s_shelf_feedback_pending = false;
    s_cover_batch_present_pending = false;
    s_reader_turn_pending = false;
    s_mode = MODE_GL16;
    return true;
}
static app_redraw_t paint_reading(app_ctx_t* ctx, enum EpdDrawMode mode) {
#if CONFIG_READER_PERF
    READER_PERF_SCOPE(READER_PERF_PAINT);
#endif
    s_water_turn_pending = false;
    s_reader_turn_pending = false;
    int64_t started = esp_timer_get_time();
    (void)ui_reader_background_prepare(app_settings_book_background_path());
    prepare_inline_image();
    s_reader_image_refresh_pending = !app_settings_reader_hide_images() &&
        book_layout_page_image_count(s_reader_session.position.page) > 0 && s_page_images;
    lock_draw();
    const uint8_t *prepared = NULL;
    if (!s_toolbar && !s_clear_confirm) {
        prepared = reader_prefetch_frame(s_reader_session.position.page);
        if (!prepared && reader_prefetch_resume(s_reader_session.position.page)) prepared = reader_prefetch_frame(s_reader_session.position.page);
    }
    bool cached = prepared != NULL;
    if (cached) {
        memcpy(ctx->fb, prepared, fb_bytes());
        // 这一页没有经过同步绘制，接管预绘时记下的内链命中表。
        // This page skipped the synchronous draw, so take over the hit table recorded during
        // its prefetch.
        book_layout_adopt_page(s_reader_session.position.page);
        draw_reader_outer(ctx->fb,s_reader_session.position.page);
    }
    else draw_reader(ctx->fb, s_reader_session.position.page);
    // 下一页可能预渲染于数分钟前，复制后以当前时间重绘状态栏。
    // A prefetched page can be minutes old; replace its status with the current time.
    if (cached && reader_status_visible()) reader_status_paint(ctx->fb);
    unlock_draw();
    ESP_LOGI(TAG, "paint cached=%d ms=%lld", cached, (esp_timer_get_time() - started) / 1000);
    s_area = reader_area();
    s_reader_footer_pending = !s_reader_fullscreen;
    s_mode = mode;
    return APP_REDRAW_AREA;
}

/* ---- 文件与进度 / Files and progress ---- */
typedef struct {
    size_t chapter;
    char** images;
    size_t count;
} reader_dims_ctx_t;
static reader_dims_ctx_t s_reader_dims_ctx;

// 按 PR9 通过完整资源解包测量，兼容 SVG 包装及较晚的 JPEG 尺寸头。/ Use PR9's full resource measurement for SVG wrappers and late JPEG headers.
static bool reader_image_dims(void* ctx, int image, int* width, int* height) {
    const reader_dims_ctx_t* c = (const reader_dims_ctx_t*)ctx;
    if (!c || image < 0 || (size_t)image >= c->count) return false;
    uint8_t* encoded = NULL;
    size_t size = 0;
    bool png = false;
    if (book_chapter_image(c->chapter, c->images[image], &encoded, &size, &png) != ESP_OK) return false;
    unsigned w = 0, h = 0;
    const bool ok = book_image_dimensions(encoded, size, png, &w, &h) && w && h;
    free(encoded);
    if (!ok) return false;
    if (width) *width = (int)w;
    if (height) *height = (int)h;
    return true;
}
static void flush_ticket_stats(void) {
    uint32_t seconds = s_stats_pending_ms / 1000;
    if (!seconds && !s_stats_pending_turns) return;
    if (book_ticket_record(seconds, s_stats_pending_turns) == ESP_OK) {
        s_stats_pending_ms -= seconds * 1000;
        s_stats_pending_turns = 0;
    }
}
static void track_ticket_stats(app_ctx_t* ctx) {
    if (s_view != READING || !s_text) { s_stats_last_ms = ctx->now_ms; return; }
    if (s_stats_last_ms > 0 && ctx->now_ms >= s_stats_last_ms &&
        ctx->now_ms - s_stats_last_ms <= 5000 &&
        ctx->now_ms - s_stats_activity_ms <= 300000 && !s_toolbar) {
        uint32_t elapsed = (uint32_t)(ctx->now_ms - s_stats_last_ms);
        s_stats_pending_ms += elapsed;
        s_session_read_ms += elapsed;
        app_reading_journal_record(s_reader_session.book.path,s_reader_session.book.title,
            s_reader_session.book.file_size,(unsigned)s_reader_session.position.chapter,elapsed,0);
    } else (void)app_reading_journal_poll();
    s_stats_last_ms = ctx->now_ms;
    if (s_stats_pending_ms >= 30000) flush_ticket_stats();
}
static void reader_bind_chapter(void) {
    const html_text_t *content = reader_session_content(&s_reader_session);
    s_text = content->utf8;
    s_text_len = content->len;
    s_blocks = content->blocks;
    s_block_count = content->count;
    s_links = content->links;
    s_link_count = content->link_count;
    s_anchors = content->anchors;
    s_anchor_count = content->anchor_count;
    s_link_strings = content->strings;
    s_images = content->images;
    s_image_count = content->image_count;
}
static void free_book(void) {
    flush_ticket_stats();
    (void)app_reading_journal_flush();
    e0470_page_turn_release();
    s_water_turn_pending = false;
    s_reader_turn_pending = s_reader_text_frame = s_reader_stable_frame = false;
    pending_progress_t* pending = pending_find(s_reader_session.book.path);
    if (pending && !pending->dirty) pending_discard(s_reader_session.book.path);
    invalidate_prep();
    book_layout_set_image_dims(NULL, NULL);
    s_reader_dims_ctx = (reader_dims_ctx_t){0};
    reader_session_release_layout(&s_reader_session);
    book_layout_set_chapter_lead(0, 0);
    book_layout_set_chapter_lead_width(0);
    s_chapter_lead_skip = s_chapter_lead_height = 0;
    s_chapter_lead_width = 0;
    s_chapter_heading_title[0] = s_chapter_heading_label[0] = 0;
    reader_session_release_chapter(&s_reader_session);
    reader_bind_chapter();
    release_page_images();
    s_reader_image_refresh_pending = false;
    s_selected_toc = SIZE_MAX;
    s_jump_offset = SIZE_MAX;
    reader_session_close(&s_reader_session);
    app_font_activate_system();
#if CONFIG_READER_PERF && CONFIG_READER_MEMORY
    reader_memory_snapshot("book_closed");
#endif
}
// 通栏之后一页位图更大：装不下就按最大空闲块等比缩一档——图小一圈也胜过整页消失。
// A full-bleed page bitmap is larger: when it does not fit, step the aspect-preserving size down
// into the largest free block. A slightly smaller illustration still beats a missing one.
static uint8_t *alloc_page_bitmap(int *width, int *height, size_t *pixels) {
    uint8_t *gray = heap_caps_malloc(*pixels, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (gray) return gray;
    size_t room = heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (room == 0) return NULL;
    for (int guard = 0; guard < 24 && *pixels > room; ++guard) {
        *width = *width > 1 ? *width * 7 / 8 : 1;
        *height = *height > 1 ? *height * 7 / 8 : 1;
        *pixels = (size_t)*width * (size_t)*height;
    }
    if (*pixels > room) return NULL;
    gray = heap_caps_malloc(*pixels, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (gray)
        ESP_LOGW(TAG, "inline illustration shrunk to %dx%d, free block %u KB",
                 *width, *height, (unsigned)(room / 1024));
    return gray;
}
static void release_page_images(void) {
    for (int i = 0; i < s_page_image_count; ++i) free(s_page_images[i].gray);
    free(s_page_images); s_page_images = NULL; s_page_image_count = 0;
    s_page_images_for = SIZE_MAX;
}
static void prepare_inline_image(void) {
    if (app_settings_reader_hide_images()) { release_page_images(); return; }
    uint32_t generation = book_layout_generation();
    if (s_page_images_for == s_reader_session.position.page && s_page_images_generation == generation) return;
    // 纯文字跨页不应作废已排好的下一页；布局代次改变或任一侧有插图时才失效。
    // Preserve the prefetched next text page; invalidate only across layout generations or image-bearing pages.
    const bool changed_layout_or_image = s_page_images_generation != generation || s_page_image_count > 0;
    release_page_images();
    s_page_images_for = s_reader_session.position.page; s_page_images_generation = generation;
    // 插图不显示没有别的症状，每条失败路径以前都是静默 continue；这里把结局记下来，
    // 免得继续靠猜。
    // A missing illustration has no other symptom and every failure path used to be a silent
    // continue; record the outcome instead of guessing.
    int count = reader_image_slots(s_reader_session.position.page);
    if (changed_layout_or_image || count > 0) invalidate_prep();
    size_t psram_before = heap_caps_get_free_size(MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (!count || count > (int)HTML_TEXT_MAX_BLOCKS) {
        ESP_LOGI(TAG, "inline page %u: %d image slots", (unsigned)s_reader_session.position.page, count);
        return;
    }
    s_page_images = heap_caps_calloc((size_t)count, sizeof(*s_page_images), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (!s_page_images) {
        // 分配失败时计数必须跟着归零：release_page_images 会按计数遍历这张表。
        // The count has to follow the table to zero: release_page_images walks it by count.
        s_page_image_count = 0;
        ESP_LOGW(TAG, "inline page %u: no PSRAM for %d slots", (unsigned)s_reader_session.position.page, count);
        return;
    }
    s_page_image_count = count;
    // 逐张解码并立即释放压缩数据；全页灰阶位图的总面积不超过插图区域。
    // Decode serially and release each encoded buffer; the page's bitmap area stays within its
    // illustration region.
    EpdRect band = book_layout_image_rect();
    size_t pixels_left = (size_t)band.width * band.height;
    int decoded = 0;
    for (int i = 0; i < count; ++i) {
        int index = -1, width = 0, height = 0;
        if (!reader_image_slot(s_reader_session.position.page, i, &index, NULL, &width, &height) ||
            index < 0 || (size_t)index >= s_image_count || width <= 0 || height <= 0) {
            ESP_LOGW(TAG, "inline %d: slot i=%d width=%d height=%d", i, index, width, height);
            continue;
        }
        uint8_t *encoded = NULL; size_t size = 0; bool png = false;
        esp_err_t err = book_chapter_image(s_reader_session.position.chapter, s_images[index], &encoded, &size, &png);
        if (err != ESP_OK) {
            ESP_LOGW(TAG, "inline %d %s: read %s", i, s_images[index], esp_err_to_name(err));
            free(encoded);
            continue;
        }
        unsigned source_w = 0, source_h = 0;
        if (book_image_dimensions(encoded, size, png, &source_w, &source_h) && source_w && source_h) {
            // 回退槽也使用真实宽高等比缩小，避免尺寸探测失败时拉伸图片。
            // Aspect-fit even fallback slots from decoded dimensions, avoiding stretched unknown-size images.
            uint64_t w = source_w, h = source_h;
            if (w > (unsigned)width) { h = h * width / w; w = width; }
            if (h > (unsigned)height) { w = w * height / h; h = height; }
            width = w ? (int)w : 1; height = h ? (int)h : 1;
            size_t pixels = (size_t)width * height;
            if (pixels > pixels_left) {
                ESP_LOGW(TAG, "inline %d: %ux%u over %u px budget",
                         i, (unsigned)width, (unsigned)height, (unsigned)pixels_left);
                free(encoded);
                continue;
            }
            uint8_t *gray = alloc_page_bitmap(&width, &height, &pixels);
            if (gray == NULL) {
                ESP_LOGW(TAG, "inline %d: no PSRAM for %u px (free %u)",
                         i, (unsigned)pixels,
                         (unsigned)heap_caps_get_free_size(MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
                free(encoded);
                continue;
            }
            if (book_image_grayscale(encoded, size, png, width, height, gray)) {
                s_page_images[i] = (reader_image_t){gray, width, height};
                pixels_left -= pixels;
                ++decoded;
            } else {
                ESP_LOGW(TAG, "inline %d: grayscale decode failed", i);
                free(gray);
            }
        } else {
            ESP_LOGW(TAG, "inline %d: unknown dimensions (%u bytes, png=%d)", i, (unsigned)size, (int)png);
        }
        free(encoded);
        vTaskDelay(1);
    }
    // 只在有图没画出来时汇总：插图缺失没有别的症状，而每次都打会把日志淹掉。
    // Summary only when something did not make it: a missing illustration has no other symptom,
    // and logging every page would drown the console.
    if (decoded < count) {
        ESP_LOGW(TAG, "inline page %u: %d/%d decoded, psram %u->%u KB", (unsigned)s_reader_session.position.page, decoded, count,
                 (unsigned)(psram_before / 1024),
                 (unsigned)(heap_caps_get_free_size(MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT) / 1024));
    }
}
typedef struct {
    size_t skip;
    unsigned height, width;
    reader_dims_ctx_t dims;
} chapter_layout_undo_t;

static void chapter_restore_images(void *ctx) {
    const chapter_layout_undo_t *undo = ctx;
    // 候选章释放前恢复图片借用。/ Restore image borrows before freeing the candidate.
    s_reader_dims_ctx = undo->dims;
}

static void chapter_restore_lead(void *ctx) {
    const chapter_layout_undo_t *undo = ctx;
    s_chapter_lead_skip = undo->skip;
    s_chapter_lead_height = undo->height;
    s_chapter_lead_width = undo->width;
    book_layout_set_chapter_lead(undo->skip, undo->height);
    book_layout_set_chapter_lead_width(undo->width);
}

static bool load_chapter_at(app_ctx_t* ctx, size_t chapter, size_t offset,
                            bool last_page, const char *anchor, size_t source_offset) {
    bool clear_toc = chapter != s_reader_session.position.chapter || (!anchor && source_offset == SIZE_MAX);
    reader_chapter_t candidate = {0};
    esp_err_t err = reader_chapter_read(&candidate, chapter, anchor, source_offset,
                                        body_rect().height, READER_HEADING_BAND_HEIGHT);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "chapter load failed: %s", esp_err_to_name(err));
        copy_text(s_message, sizeof(s_message), "加载失败，请检查存储卡后重试");
        return false;
    }
    chapter_layout_undo_t undo = {
        .skip = s_chapter_lead_skip, .height = s_chapter_lead_height,
        .width = s_chapter_lead_width, .dims = s_reader_dims_ctx,
    };
    reader_chapter_rollback_t rollback = {
        .ctx = &undo, .restore_images = chapter_restore_images,
        .restore_lead = chapter_restore_lead,
    };
    lock_draw();
    invalidate_prep();
    // 题头几何按当前模式重算，横排留顶部行高、竖排留右侧列宽，绘制与预留不会错开。
    // Recompute the heading geometry for the current mode -- rows at the top horizontally,
    // columns at the right vertically -- so drawing and reservation cannot drift apart.
    apply_chapter_lead(candidate.heading, candidate.label, candidate.lead_skip, candidate.lead_height);
    s_reader_dims_ctx = (reader_dims_ctx_t){chapter, candidate.content.images, candidate.content.image_count};
    book_layout_set_image_dims(reader_image_dims, &s_reader_dims_ctx);
    reader_chapter_result_t result = reader_chapter_apply_locked(&s_reader_session, &candidate,
                                                                body_rect(), s_px, &rollback);
    if (result != READER_CHAPTER_APPLIED) {
        unlock_draw();
        if (result == READER_CHAPTER_LOST_LAYOUT) {
            free_book();
            s_view = SHELF;
            ctx->leaf = 0;
        }
        copy_text(s_message, sizeof(s_message), "排版失败：内存不足或章节过长");
        return false;
    }
    reader_bind_chapter();
    release_page_images();
    s_reader_session.position.chapter = chapter;
    if (clear_toc) s_selected_toc = SIZE_MAX;
    s_chapter_lead_skip = candidate.lead_skip;
    s_chapter_lead_row_height = candidate.lead_height;
    copy_text(s_chapter_heading_title, sizeof(s_chapter_heading_title), candidate.heading);
    copy_text(s_chapter_heading_label, sizeof(s_chapter_heading_label), candidate.label);
    s_reader_session.position.page = last_page ? book_layout_page_count() - 1 :
             book_layout_page_for_offset(anchor || source_offset != SIZE_MAX ? candidate.anchor_offset : offset);
    s_jump_offset = anchor || source_offset != SIZE_MAX ? candidate.anchor_offset : SIZE_MAX;
    s_jump_page = s_reader_session.position.page;
    if (anchor || source_offset != SIZE_MAX)
        ESP_LOGI(TAG, "toc target anchor=%d source=%zu mapped=%zu page=%u/%u",
                 anchor != NULL, source_offset, candidate.anchor_offset,
                 (unsigned)s_reader_session.position.page + 1,
                 (unsigned)book_layout_page_count());
    if (book_chapter_title(chapter, s_reader_session.book.chapter_title, sizeof(s_reader_session.book.chapter_title)) != ESP_OK)
        snprintf(s_reader_session.book.chapter_title, sizeof(s_reader_session.book.chapter_title), "第 %u 节", (unsigned)chapter + 1);
    copy_text(s_font_path, sizeof(s_font_path), ttf_font_path());
    s_message[0] = 0;
    unlock_draw();
    prepare_inline_image();
    // 开书这条链上有解析、解压和字体装载，栈曾经在这里溢出过；留一个可观测的水位。
    // 顺带量一下章节排完后的 PSRAM 余量：书内字体的装载预算要靠这个数来定，不能靠估。
    // Parsing, inflate and face loading all ride this chain, and it overflowed once before;
    // keep the headroom observable. The PSRAM left after a chapter is laid out is also the
    // number the face budget should be derived from rather than guessed.
    ESP_LOGI(TAG, "chapter %u stack free %u psram free %u largest %u", (unsigned)chapter,
             (unsigned)uxTaskGetStackHighWaterMark(NULL),
             (unsigned)heap_caps_get_free_size(MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT),
             (unsigned)heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
    return true;
}
static bool load_chapter(app_ctx_t* ctx, size_t chapter, size_t offset, bool last_page) {
    return load_chapter_at(ctx, chapter, offset, last_page, NULL, SIZE_MAX);
}
// 关闭图片时跳过纯插图页，避免出现空白翻页。/ Skip illustration-only pages when images are disabled.
static void skip_hidden_image_pages(app_ctx_t *ctx, int direction) {
    if (!app_settings_reader_hide_images()) return;
    for (int guard = 0; guard < 128 && book_layout_page_image(s_reader_session.position.page) >= 0; ++guard) {
        if (direction > 0 && s_reader_session.position.page + 1 < book_layout_page_count()) ++s_reader_session.position.page;
        else if (direction < 0 && s_reader_session.position.page > 0) --s_reader_session.position.page;
        else if (direction > 0 && s_reader_session.position.chapter + 1 < book_chapter_count()) {
            if (!load_chapter(ctx, s_reader_session.position.chapter + 1, 0, false)) break;
        } else if (direction < 0 && s_reader_session.position.chapter > 0) {
            if (!load_chapter(ctx, s_reader_session.position.chapter - 1, 0, true)) break;
        } else break;
    }
}
static void reader_open_replace(void) {
    save_progress();
    free_book();
    s_reader_fullscreen = s_requested_fullscreen;
    app_font_activate_reading();
}
static void reader_open_describe(const char *path) {
    // 书源打开后再把书内字体开关接上：EPUB 才有效，TXT 是空操作。
    // Wire the embedded-face switch only after the source is open: it only applies to EPUB,
    // and is a no-op for TXT.
    s_book_fonts_applied = app_settings_book_fonts();
    book_set_embedded_fonts(s_book_fonts_applied);
    copy_text(s_reader_session.book.path, sizeof(s_reader_session.book.path), path);
    const char *filename = strrchr(path, '/');
    filename = filename ? filename + 1 : path;
    if (!book_title_from_path(path, s_reader_session.book.title, sizeof(s_reader_session.book.title)))
        clean_filename(s_reader_session.book.title, sizeof(s_reader_session.book.title), filename);
}
static bool reader_open_load(void *ctx, size_t chapter, size_t offset) {
    if (!load_chapter(ctx, chapter, offset, false)) return false;
    skip_hidden_image_pages(ctx, 1);
    return s_text != NULL;
}
static bool open_book_impl(app_ctx_t* ctx, const char* path) {
#if CONFIG_READER_PERF
    reader_perf_begin("open");
    READER_PERF_SCOPE(READER_PERF_ACTION);
#endif
    if (!book_file_path_supported(path)) {
        copy_text(s_message, sizeof(s_message), "此版本仅支持 TXT、EPUB 和 MOBI 书籍");
        return false;
    }
    s_reader_return_home = false;
    if (!strncmp(path, "/sdcard/", 8)) {
        board_sd_info_t sd = {0};
        board_sd_get_info(&sd);
        if (!sd.present || !sd.mounted) {
            copy_text(s_message, sizeof(s_message), "TF 卡不可用，请重新挂载后打开");
            return false;
        }
    }
    // 自动续读也不能绕过同路径旧文件的清理重试。
    // Automatic resume must also finish cleanup of the old file at this path.
    if (delete_retry_find(path)) {
        copy_text(s_message, sizeof(s_message), "文件已删除，进度清理失败，请重试");
        return false;
    }
    struct stat st;
    if (stat(path, &st) != 0 || !S_ISREG(st.st_mode) || st.st_size < 0 || (uint64_t)st.st_size > UINT32_MAX) {
        copy_text(s_message, sizeof(s_message), "文件不可读，请检查 TF 卡和书籍文件");
        return false;
    }
    if (strncmp(path, "/flash/", 7) == 0 && st.st_size > BOOK_STORE_FLASH_FILE_MAX) {
        copy_text(s_message, sizeof(s_message), "内置存储单本限 1 MB");
        return false;
    }
    wifi_transfer_status_t network = {0};
    wifi_transfer_get_status(&network);
    s_shelf_key_mode=s_shelf_key_paint_pending=false;s_shelf_key_delta=s_shelf_key_pressed=0;
    // 书架封面和上一本书的预渲染缓冲可以重建；先释放，再解析 EPUB。
    // Cover and previous-page caches are rebuildable; release them before EPUB parsing.
    invalidate_covers();
    lock_draw();
    reader_prefetch_discard();
    unlock_draw();
    if (network.network_ready) ttf_font_cache_clear();
    // 开书前把字形缓存全还回去并量一下能腾多少：书内字体要的正是这块 PSRAM，而系统字体
    // 的缓存是里面最容易回收的。字体本身保持装载，翻页时按需重新缓存。
    // Hand every glyph cache back before opening and measure what that was worth: embedded
    // faces need exactly this PSRAM, and the system face's cache is the easiest part to
    // reclaim. The faces stay loaded and re-cache on demand.
    size_t psram_before = heap_caps_get_free_size(MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    size_t reclaimed = ttf_font_cache_clear_all();
    size_t psram_after = heap_caps_get_free_size(MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    ESP_LOGI(TAG, "open start wifi=%d internal=%u/%u psram=%u/%u cache=%uK freed=%uK",
             network.network_ready,
             (unsigned)heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT),
             (unsigned)heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT),
             (unsigned)psram_after,
             (unsigned)heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT),
             (unsigned)(reclaimed / 1024),
             (unsigned)((psram_after - psram_before) / 1024));
    reader_open_t open = {
        .session = &s_reader_session, .px = &s_px,
        .reserve = pending_reserve, .replace = reader_open_replace,
        .describe = reader_open_describe, .pending_restore = pending_restore,
        .load = reader_open_load, .close = free_book, .load_ctx = ctx,
    };
    esp_err_t err = ESP_OK;
    reader_open_result_t result = reader_open_run(&open, path, (uint32_t)st.st_size, &err);
    if (result == READER_OPEN_RESERVE_OLD) {
        copy_text(s_message, sizeof(s_message), "内存不足，无法保留待保存进度");
        return false;
    }
    if (result == READER_OPEN_RESERVE_NEW) {
        copy_text(s_message, sizeof(s_message), "内存不足，无法打开图书");
        app_font_activate_system();
        return false;
    }
    if (result == READER_OPEN_SOURCE_FAILED) {
        pending_progress_t* pending = pending_find(path);
        if (pending && !pending->dirty) pending_discard(path);
        copy_text(s_message, sizeof(s_message),
                  err == ESP_ERR_NOT_SUPPORTED ? "文件格式或压缩方式暂不支持" :
                  err == ESP_ERR_NO_MEM ? "内存不足，请重启后重试" :
                  err == ESP_ERR_INVALID_SIZE ? "图书资源过大或章节过多" :
                  "无法打开图书，请检查文件");
        ESP_LOGW(TAG, "open failed: %s internal=%u/%u psram=%u/%u", esp_err_to_name(err),
                 (unsigned)heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT),
                 (unsigned)heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT),
                 (unsigned)heap_caps_get_free_size(MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT),
                 (unsigned)heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
        app_font_activate_system();
        return false;
    }
    if (result == READER_OPEN_CONTENT_FAILED) return false;
    s_view = READING;
    s_reader_entry_full = true;
    s_reader_image_refresh_pending = !app_settings_reader_hide_images() &&
        book_layout_page_image_count(s_reader_session.position.page) > 0 && s_page_images;
    s_stats_last_ms = s_stats_activity_ms = ctx->now_ms;
    app_reading_journal_record(s_reader_session.book.path,s_reader_session.book.title,
        s_reader_session.book.file_size,(unsigned)s_reader_session.position.chapter,0,0);
    s_reader_panel = READER_PANEL_NONE;
    s_clear_confirm = s_batch_confirm = false;
    s_session_read_ms = s_session_turns = 0;
    s_reader_notice[0] = 0;
    s_turns = s_unsaved = 0;
    pending_mark_latest(s_reader_session.book.path);
    save_progress();
    // 打开即重新上书架：移出书架只隐藏条目，重新读这本书就是它该在架上的信号。
    // Opening puts the book back on the shelf: removal only hides the entry, and reading it
    // again is the signal that it belongs there.
    if (shelf_hidden_load(s_reader_session.book.path)) (void)shelf_hidden_save(s_reader_session.book.path, false);
    ESP_LOGI(TAG, "opened kind=%d chapters=%u pages=%u px=%d", book_kind(), (unsigned)book_chapter_count(), (unsigned)book_layout_page_count(), s_px);
    return true;
}
static bool open_book(app_ctx_t *ctx, const char *path) {
    if (book_category_pending(app_settings_books_dir())) {
        copy_text(s_message,sizeof(s_message),"分类整理未完成，请在管理分类中重试恢复"); return false;
    }
    if (!book_cover_jobs_quiesce(2000)) {
        copy_text(s_message,sizeof(s_message),"封面任务正在收尾，请稍后再试");return false;
    }
    esp_err_t checkpoint_error=pico_boot_book_begin(path);
    if (checkpoint_error != ESP_OK) {
        ESP_LOGW("book", "open checkpoint failed: %s", esp_err_to_name(checkpoint_error));
        copy_text(s_message, sizeof(s_message), "启动记录保存失败，请重试");
        return false;
    }
    bool ok = open_book_impl(ctx, path);
    pico_boot_book_end(ok);
    return ok;
}
static bool open_requested_book(app_ctx_t* ctx) {
    if (!s_requested_open[0]) return false;
    char path[BOOK_STORE_PATH_MAX];
    copy_text(path, sizeof(path), s_requested_open);
    bool from_home = s_requested_open_home;
    s_requested_open[0] = 0;
    s_requested_open_home = false;
    bool opened = open_book(ctx, path);
    s_requested_fullscreen = app_settings_reader_open_fullscreen();
    if (!opened) return false;
    s_reader_return_home = from_home;
    return true;
}
/* ---- 书架数据 / Shelf data ---- */
static int shelf_category_add(const char *path) {
    (void)book_shelf_categories_add(&s_legacy_categories,path);
    return book_shelf_categories_add(&s_shelf_categories, path);
}
static void shelf_category_init(void) {
    if (book_shelf_categories_init(&s_shelf_categories, app_settings_books_dir())) s_shelf_cache_valid = false;
    book_shelf_categories_init(&s_legacy_categories,app_settings_books_dir());
}
static const char *shelf_category_name(int index) {
    return book_shelf_categories_name(&s_shelf_categories, index);
}
static book_shelf_query_t shelf_query(void) {
    return book_shelf_categories_query(&s_shelf_categories, s_filter, s_recent_sort);
}
static void shelf_category_remember(app_ctx_t *ctx) {
    if (s_view == SHELF && !s_scan_pending)
        book_shelf_categories_remember(&s_shelf_categories, app_settings_shelf_style() == 5, ctx->leaf);
}
static void shelf_category_restore(app_ctx_t *ctx) {
    ctx->leaf = book_shelf_categories_restore(&s_shelf_categories, app_settings_shelf_style() == 5,
                                             s_visible_count, shelf_rows());
}
static void shelf_categories_recount(void) {
    book_collection_apply(&s_collection,s_shelf,s_count);
    book_shelf_categories_recount(&s_shelf_categories, s_shelf, s_count);
}
static int shelf_category_at(int ordinal) {
    return book_shelf_categories_at(&s_shelf_categories, ordinal);
}
static int shelf_category_available(void) {
    return book_shelf_categories_available(&s_shelf_categories);
}
static EpdRect shelf_category_box(void) {
    const int menu_page=book_shelf_categories_menu_page(&s_shelf_categories);
    int count=shelf_category_available()-menu_page*SHELF_CATEGORY_ROWS;
    if (count>SHELF_CATEGORY_ROWS) count=SHELF_CATEGORY_ROWS;
    if (count<1) count=1;
    return (EpdRect){36,s_shelf_selecting?272:158,400,16+count*83+128+(shelf_category_available()>SHELF_CATEGORY_ROWS ? 56 : 0)};
}
static void draw_category_menu(uint8_t *fb) {
    const int menu_page=book_shelf_categories_menu_page(&s_shelf_categories);
    EpdRect box=shelf_category_box();
    ui_fill_round_rect(fb,box,23,UI_GRAY_WHITE);
    ui_draw_control_frame(fb,box,23,0x50);
    for (int row=0;row<SHELF_CATEGORY_ROWS;++row) {
        int index=shelf_category_at(menu_page*SHELF_CATEGORY_ROWS+row);
        if (index<0) break;
        int y=box.y+8+row*83;
        bool selected=index==book_shelf_categories_selected(&s_shelf_categories);
        if (selected) ui_fill_round_rect(fb,(EpdRect){box.x+8,y,box.width-16,79},16,0xd0);
        ui_draw_icon(fb,box.x+35,y+40,27,index<2 ? UI_ICON_LIBRARY_BIG : UI_ICON_FOLDER_OPEN,UI_GRAY_BLACK);
        char name[BOOK_STORE_PATH_MAX],count[24];
        snprintf(count,sizeof(count),"%d",book_shelf_categories_count(&s_shelf_categories,index));
        int name_width=290-ui_text_fixed_width_px(ui_text_effective_px(22),count);
        if (name_width>228) name_width=228;
        if (name_width<60) name_width=60;
        copy_text(name,sizeof(name),shelf_category_name(index)); fit_text(name,28,name_width);
        ui_text_vc(fb,box.x+62,y+40,28,name,EPD_DRAW_ALIGN_LEFT,false);
        ui_text_vc(fb,box.x+box.width-38,y+40,22,count,EPD_DRAW_ALIGN_RIGHT,false);
        if (selected) epd_fill_circle(box.x+box.width-20,y+40,4,UI_GRAY_BLACK,fb);
    }
    if (shelf_category_available()>SHELF_CATEGORY_ROWS) {
        int y=box.y+box.height-128-28;
        ui_draw_icon(fb,box.x+48,y,28,UI_ICON_CHEVRON_LEFT,menu_page ? 0 : 0x90);
        char pages[24]; snprintf(pages,sizeof(pages),"%d / %d",menu_page+1,
            (shelf_category_available()+SHELF_CATEGORY_ROWS-1)/SHELF_CATEGORY_ROWS);
        ui_text_vc(fb,box.x+box.width/2,y,22,pages,EPD_DRAW_ALIGN_CENTER,false);
        ui_draw_icon(fb,box.x+box.width-48,y,28,UI_ICON_CHEVRON_RIGHT,0);
    }
    ui_hairline(fb,box.y+box.height-128,box.x+16,box.width-32,0xb0);
    shelf_label(fb,(EpdRect){box.x+16,box.y+box.height-128,box.width-32,64},26,"新建分类",true);
    shelf_label(fb,(EpdRect){box.x+16,box.y+box.height-64,box.width-32,64},26,"管理分类",true);
}
static void sort_shelf(app_ctx_t* ctx) {
    invalidate_covers();
    book_shelf_query_t query = shelf_query();
    book_shelf_categories_set_sort(&s_shelf_categories,query.recent_sort);
    s_visible_count = book_shelf_sort(s_shelf, s_count, &query);
    ctx->leaf = 0;
}
static app_redraw_t shelf_category_gesture(app_ctx_t *ctx, const ui_gesture_event_t *ev) {
    if (ev->type!=UI_GESTURE_TAP && ev->type!=UI_GESTURE_SWIPE_U && ev->type!=UI_GESTURE_SWIPE_D)
        return APP_REDRAW_NONE;
    EpdRect previous=shelf_category_box();
    if (!s_shelf_selecting && ev->type==UI_GESTURE_TAP && ui_nav_hit(ev->x0,ev->y0)>=0) {
        book_shelf_categories_menu_close(&s_shelf_categories); ui_nav_request(ctx,ui_nav_hit(ev->x0,ev->y0)); return APP_REDRAW_NONE;
    }
    if (!book_shelf_categories_menu_is_open(&s_shelf_categories)) {
        if (ev->type!=UI_GESTURE_TAP || !book_shelf_categories_menu_open(&s_shelf_categories)) return APP_REDRAW_NONE;
    } else {
        const int menu_page=book_shelf_categories_menu_page(&s_shelf_categories);
        int pages=(shelf_category_available()+SHELF_CATEGORY_ROWS-1)/SHELF_CATEGORY_ROWS;
        int direction=ev->type==UI_GESTURE_SWIPE_U ? 1 : ev->type==UI_GESTURE_SWIPE_D ? -1 : 0;
        if (!direction && ui_rect_hit(previous,ev->x0,ev->y0)) {
            int row=((int)ev->y0-previous.y-8)/83;
            if (ev->y0>=previous.y+previous.height-128) {
                bool create=ev->y0<previous.y+previous.height-64;
                book_shelf_categories_menu_close(&s_shelf_categories);
                category_begin(ctx,false,false);
                if(create&&!s_category_ui.blocked)book_category_ui_edit(&s_category_ui,&s_shelf_categories,false);
                return APP_REDRAW_PAGE;
            }
            bool footer=pages>1 && ev->y0>=previous.y+previous.height-128-56;
            if (footer) direction=ev->x0<previous.x+previous.width/2 ? -1 : 1;
            else {
                int index=ev->y0>=previous.y+8 && row<SHELF_CATEGORY_ROWS
                    ? shelf_category_at(menu_page*SHELF_CATEGORY_ROWS+row) : -1;
                if (index>=0) {
                    shelf_category_remember(ctx); (void)book_shelf_categories_select(&s_shelf_categories,index);
                    if(!s_shelf_selecting)clear_selection();
                    sort_shelf(ctx); shelf_category_restore(ctx);
                    return APP_REDRAW_PAGE;
                }
            }
        } else if (!direction) book_shelf_categories_menu_close(&s_shelf_categories);
        if (direction) {
            if (!book_shelf_categories_menu_move(&s_shelf_categories,direction)) return APP_REDRAW_NONE;
        }
    }
    EpdRect next=shelf_category_box();
    int bottom=previous.y+previous.height>next.y+next.height ? previous.y+previous.height : next.y+next.height;
    s_area=(EpdRect){24,79,424,bottom-79+4}; s_mode=MODE_GL16;
    render(ctx,ctx->fb);
    return APP_REDRAW_AREA;
}
static bool shelf_scan_cancel(void *ctx) {
    return s_view == SHELF && app_poll_main_navigation(ctx);
}
static book_shelf_scan_t shelf_scan_context(app_ctx_t *ctx) {
    return (book_shelf_scan_t){
        .catalog = {&s_shelf, &s_count, &s_shelf_capacity}, .query = s_query,
        .cancel = shelf_scan_cancel, .cancel_ctx = ctx, .category_add = shelf_category_add,
        .favorite_read = favorite_read_handle, .hidden_read = shelf_hidden_read_handle,
    };
}
// 缓存返回也补入书根以外读过的书，保留原调用时机。
// Backfill outside-root reads on cached returns too, retaining the original call timing.
static bool shelf_backfill_read_books(bool *truncated) {
    book_shelf_scan_t scan = shelf_scan_context(NULL);
    return book_shelf_scan_backfill(&scan, truncated);
}

static bool legacy_category_recover(void) {
    if (book_category_pending(app_settings_books_dir())) {
        board_sd_info_t recovery_sd={0};board_sd_get_info(&recovery_sd);
        if (!recovery_sd.mounted || usb_storage_active() || !wifi_transfer_try_stop_if_idle() ||
            !book_cover_jobs_quiesce(2000) || book_category_recover(app_settings_books_dir(),category_extra_state)!=ESP_OK) return false;
        pending_drop_invalidated(); book_store_notify_changed(); book_lock_collage_invalidate();
    }
    return true;
}
static void scan_shelf(app_ctx_t* ctx) {
    bool recovered_legacy = legacy_category_recover();
    const uint32_t progress_revision=book_progress_revision();
    shelf_category_init();
    book_shelf_categories_begin_scan(&s_shelf_categories);
    book_shelf_categories_begin_scan(&s_legacy_categories);
    s_count = 0;
    s_visible_count = 0;
    s_message[0] = 0;
    s_shelf_warning[0] = 0;
    // 内置分区每次启动都必须重新挂载；已有文件不会因此格式化。
    // Flash mounts on each boot; existing files are not formatted by that mount.
    book_store_root_t roots[BOOK_STORE_ROOT_MAX];
    int n = 0;
    esp_err_t root_err = book_store_roots(roots, &n);
    bool truncated = false, unreadable = root_err != ESP_OK || book_store_roots_degraded(), skipped = false;
    nvs_handle_t favorites = 0;
    nvs_handle_t hidden = 0;
    (void)nvs_open("rp_favs", NVS_READONLY, &favorites);
    (void)nvs_open(SHELF_HIDDEN_NS, NVS_READONLY, &hidden);
    s_storage[0] = 0;
    book_shelf_scan_t scan = shelf_scan_context(ctx);
    for (int i = 0; i < n; ++i) {
        if (i == 0 || roots[i].is_flash || roots[i - 1].is_flash) {
            char capacity[64];
            snprintf(capacity, sizeof(capacity), "%s%s %.1f MB", s_storage[0] ? " · " : "",
                     roots[i].is_flash ? "内置余" : "TF余", book_store_free_bytes(&roots[i]) / 1048576.0);
            strncat(s_storage, capacity, sizeof(s_storage) - strlen(s_storage) - 1);
        }
        if (s_view == SHELF && app_poll_main_navigation(ctx)) break;
        book_shelf_scan_dir(&scan, roots[i].path, roots[i].is_flash, favorites, hidden,
                            &truncated, &unreadable, &skipped);
    }
    if (ctx->request_app) {
        if (favorites) nvs_close(favorites);
        if (hidden) nvs_close(hidden);
        s_shelf_cache_valid = false; s_scan_pending = true;
        return;
    }
    // 同路径重新出现也先完成旧清理，避免新阅读进度被后续重试擦掉。
    // Finish old cleanup even if the path reappears, before new reading can create progress.
    book_shelf_scan_retries(&scan, s_delete_retries, &truncated);
    if (favorites) nvs_close(favorites);
    if (hidden) nvs_close(hidden);
    // 补齐书根目录之外读过的书；显式移出书架的除外。
    // Backfill books read outside the roots, minus the ones removed from the shelf.
    (void)shelf_backfill_read_books(&truncated);
    if (recovered_legacy) (void)shelf_collection_sync(!unreadable && !truncated && !skipped);
    shelf_categories_recount();
    sort_shelf(ctx);
    if (s_view==SHELF) shelf_category_restore(ctx);
    s_latest_path[0] = 0;
    char last_path[BOOK_STORE_PATH_MAX];
    if (book_progress_last_path(last_path, sizeof(last_path))) {
        for (int i = 0; i < s_count; ++i) {
            if (!strcmp(s_shelf[i].path, last_path) && !s_shelf[i].removed) {
                copy_text(s_latest_path, sizeof(s_latest_path), last_path);
                break;
            }
        }
    }
    if (!n) copy_text(s_storage, sizeof(s_storage), "存储不可用，请检查 TF 卡");
    if (truncated) snprintf(s_shelf_warning, sizeof(s_shelf_warning), "内存不足，仅列 %d 本；释放后重扫", s_count);
    else if (unreadable) copy_text(s_shelf_warning, sizeof(s_shelf_warning), "部分目录或文件不可读，请检查后重扫");
    else if (skipped) copy_text(s_shelf_warning, sizeof(s_shelf_warning), "部分文件过大或名称过长，未列入");
    else if (s_pending_invalidated) copy_text(s_shelf_warning, sizeof(s_shelf_warning), "图书已更新，旧待保存进度已作废");
    if (!recovered_legacy) copy_text(s_shelf_warning,sizeof(s_shelf_warning),"旧分类整理未完成，请在管理分类中重试恢复");
    if (!s_count && !unreadable && !truncated) copy_text(s_message, sizeof(s_message), "暂无图书，请传入 TXT、EPUB 或 MOBI");
    board_sd_info_t cache_sd = {0};
    board_sd_get_info(&cache_sd);
    copy_text(s_cache_library,sizeof(s_cache_library),cache_sd.mounted?cache_sd.identity:"builtin");
    s_cache_sd_present = cache_sd.present;
    s_cache_sd_mounted = cache_sd.mounted;
    s_cache_shelf_style = app_settings_shelf_style();
    s_store_revision = book_store_revision();
    s_progress_revision = progress_revision;
    s_shelf_cache_valid = !unreadable && !truncated;
    ESP_LOGI(TAG, "shelf books=%d roots=%d", s_count, n);
}

/* ---- 分类管理 / Category management ---- */
static void shelf_label(uint8_t *fb,EpdRect rect,int px,const char *value,bool center) {
    char line[288];copy_text(line,sizeof(line),value);
    px=ui_text_effective_px(px);if(px>rect.height-8)px=rect.height-8;
    if (center) {
        while (px > 12 && ui_text_fixed_width_px(px,line)>rect.width) --px;
    } else {
        while(*line && ui_text_fixed_width_px(px,line)>rect.width){size_t n=strlen(line)-1;while(n&&((unsigned char)line[n]&0xc0)==0x80)--n;line[n]=0;}
    }
    ui_text_fixed_vc(fb,center?rect.x+rect.width/2:rect.x,rect.y+rect.height/2,px,line,center?EPD_DRAW_ALIGN_CENTER:EPD_DRAW_ALIGN_LEFT,false);
}
static esp_err_t category_extra_state(const char *old_path, const char *new_path, uint32_t size) {
    return app_reading_journal_relocate(old_path,new_path,size) ? ESP_OK : ESP_FAIL;
}
static bool shelf_collection_load(bool retry) {
    board_sd_info_t sd = {0}; board_sd_get_info(&sd);
    if (sd.present && (!sd.mounted || !sd.identity[0])) return false;
    if (book_store_prepare_data() != ESP_OK) return false;
    char prefix[BOOK_STORE_PATH_MAX];
    snprintf(prefix,sizeof(prefix),"/flash/.mogo-shelf-%s",sd.mounted?sd.identity:"builtin");
    if (!strcmp(s_collection.prefix,prefix) && !retry) return s_collection.ready;
    if (strcmp(s_collection.prefix,prefix)) {
        book_shelf_categories_dispose(&s_shelf_categories);
        book_shelf_categories_init(&s_shelf_categories,app_settings_books_dir());
        s_category_result[0]=0;
    }
    s_collection_bound_generation=0;
    esp_err_t err=book_collection_open(&s_collection,prefix);
    return err==ESP_OK;
}
static bool shelf_collection_sync(bool complete) {
    if (!shelf_collection_load(false)) {
        copy_text(s_shelf_warning,sizeof(s_shelf_warning),"分类资料暂不可用，仍可浏览全部书籍");
        return false;
    }
    const char *folders[BOOK_COLLECTION_CATEGORIES]; size_t n=0;
    for(int ordinal=2,index;(index=book_shelf_categories_at(&s_legacy_categories,ordinal))>=0 && n<BOOK_COLLECTION_CATEGORIES;++ordinal)
        folders[n++]=book_shelf_categories_path(&s_legacy_categories,index);
    esp_err_t err=book_collection_sync(&s_collection,app_settings_books_dir(),folders,n,s_shelf,s_count,complete);
    if(err!=ESP_OK) copy_text(s_shelf_warning,sizeof(s_shelf_warning),"分类保存失败，原归属已保留");
    if(book_collection_migrated(&s_collection) &&
       (!book_shelf_categories_logical(&s_shelf_categories) || s_collection_bound_generation!=s_collection.generation)) {
        if(!book_shelf_categories_bind(&s_shelf_categories,&s_collection)) {
            copy_text(s_shelf_warning,sizeof(s_shelf_warning),"分类已保存，内存不足，稍后重试显示");
            (void)book_shelf_categories_select(&s_shelf_categories,0);
            return false;
        }
        s_collection_bound_generation=s_collection.generation;
    }
    if(s_collection.recovered)copy_text(s_shelf_warning,sizeof(s_shelf_warning),"分类已从备用副本恢复，请核对");
    return err==ESP_OK && book_shelf_categories_logical(&s_shelf_categories);
}
static void category_reset(void) {
    book_category_ui_end(&s_category_ui);
    book_shelf_covers_cancel(&s_category_covers); book_shelf_covers_release(&s_category_covers);
    s_category_cover_key=0; s_category_quick=false;
}
static void category_begin(app_ctx_t *ctx, bool picker, bool bulk) {
    shelf_category_remember(ctx); book_shelf_categories_menu_close(&s_shelf_categories);
    s_category_parent=s_view; s_category_bulk=bulk;
    category_reset(); book_category_ui_begin(&s_category_ui,picker);
    s_category_ui.books=s_shelf; s_category_ui.count=s_count; s_category_ui.cover=category_cover_draw;
    s_category_ui.can_undo=book_collection_can_undo(&s_collection);
    s_category_ui.blocked=!book_shelf_categories_logical(&s_shelf_categories) || s_collection_bound_generation!=s_collection.generation || !s_collection.ready || s_collection.blocked ||
        book_category_pending(app_settings_books_dir());
    if(s_category_ui.blocked)copy_text(s_category_ui.notice,sizeof(s_category_ui.notice),"分类资料不可用，原书籍不受影响");
    if(picker && !bulk) {
        clear_selection();
        for(int i=0;i<s_count;++i)if(!strcmp(s_shelf[i].path,s_managed.path))s_shelf[i].selected=true;
    }
    s_view=CATEGORIES; s_pressed_control=-1;
    book_shelf_covers_cancel(&s_shelf_covers);
}
static void category_start_books(app_ctx_t *ctx,uint32_t target,bool review) {
    if(s_view!=CATEGORIES)category_begin(ctx,false,review);
    if(s_category_ui.blocked)return;
    if(!review)clear_selection();
    s_category_bulk=true;
    book_category_ui_books(&s_category_ui,target,review);
}
static void category_cover_draw(uint8_t *fb,int row,int index,EpdRect rect) {
    const book_shelf_cover_t *cover=book_shelf_covers_slot(&s_category_covers,(unsigned)row);
    if(cover->index==index && cover->gray && !book_shelf_covers_pending(&s_category_covers)) {
        unsigned width=cover->width?cover->width:BOOK_COVER_W,height=cover->height?cover->height:BOOK_COVER_H;
        for(int y=0;y<rect.height;++y)for(int x=0;x<rect.width;++x)
            epd_draw_pixel(rect.x+x,rect.y+y,ui_image_dither_gray(ui_contrast_gray(cover->gray[(size_t)(y*height/rect.height)*width+x*width/rect.width]),rect.x+x,rect.y+y),fb);
    } else ui_draw_control_frame(fb,rect,0,0x90);
}
static bool category_prepare_covers(app_ctx_t *ctx) {
    if(s_category_ui.screen!=CATEGORY_BOOKS || (ctx->touch && ctx->touch->touched))return false;
    int rows=book_category_ui_rows(true); uint32_t key=2166136261u;
    s_category_ui.page=book_shelf_saved_page(s_category_ui.page,book_category_ui_book_count(&s_category_ui),rows);
    for(int row=0;row<rows;++row)key=(key^(uint32_t)(book_category_ui_book_at(&s_category_ui,s_category_ui.page*rows+row)+1))*16777619u;
    if(key!=s_category_cover_key) {
        book_shelf_covers_cancel(&s_category_covers); book_shelf_covers_release(&s_category_covers);
        s_category_cover_key=key;
    }
    for(int row=0;row<rows;++row) {
        int index=book_category_ui_book_at(&s_category_ui,s_category_ui.page*rows+row);
        book_shelf_covers_prepare(&s_category_covers,row,index,index>=0,false,0,0,ctx->now_ms);
        if(index<0 || !(book_shelf_covers_pending(&s_category_covers)&(1u<<row)))continue;
        book_cover_job_t request={.width=64,.height=88};
        copy_text(request.path,sizeof(request.path),s_shelf[index].path);
        copy_text(request.title,sizeof(request.title),s_shelf[index].name);
        copy_text(request.author,sizeof(request.author),s_shelf[index].author);
        (void)book_shelf_covers_collect(&s_category_covers,row,&request);
    }
    return book_shelf_covers_reveal(&s_category_covers,true)!=0;
}
static void category_draw(uint8_t *fb) { book_category_ui_draw(&s_category_ui,&s_shelf_categories,fb); }
static void category_select(app_ctx_t *ctx,uint32_t id) {
    int index=book_shelf_categories_index(&s_shelf_categories,id);
    (void)book_shelf_categories_select(&s_shelf_categories,index>=0?index:1);
    sort_shelf(ctx); shelf_category_restore(ctx);
}
static void category_close(app_ctx_t *ctx) {
    bool adding=s_category_ui.screen==CATEGORY_BOOKS && !s_category_ui.selecting_only;
    uint32_t target=s_category_ui.destination;
    book_view_t parent=s_category_parent;
    bool keep=s_category_bulk && !adding;
    category_reset(); s_view=adding?SHELF:parent;
    if(!keep)clear_selection();
    if(adding){s_shelf_selecting=false;category_select(ctx,target);}
    else if(s_view==SHELF)shelf_category_restore(ctx);
}
static app_redraw_t category_back(app_ctx_t *ctx) {
    if(!book_category_ui_back(&s_category_ui))return APP_REDRAW_PAGE;
    category_close(ctx); return APP_REDRAW_PAGE;
}
static app_redraw_t category_input_paint(app_ctx_t *ctx) {
    ui_keyboard_update_t update=book_category_ui_input_paint(&s_category_ui,ctx->fb);
    s_area=update.area; s_mode=update.layout?MODE_GL16:MODE_DU; s_input_settle=update.settle;
    return update.area.width?APP_REDRAW_AREA:APP_REDRAW_NONE;
}
static app_redraw_t category_gesture(app_ctx_t *ctx,const ui_gesture_event_t *ev) {
    book_category_screen_t before=s_category_ui.screen;
    char notice[128]; copy_text(notice,sizeof(notice),s_category_ui.notice);
    int result=book_category_ui_gesture(&s_category_ui,&s_shelf_categories,ev,ctx->now_ms);
    if(result==2){category_close(ctx);return APP_REDRAW_PAGE;}
    if(result && before==CATEGORY_EDIT && s_category_ui.screen==CATEGORY_EDIT &&
        s_category_ui.command==CATEGORY_IDLE && !strcmp(notice,s_category_ui.notice))return category_input_paint(ctx);
    return result?APP_REDRAW_PAGE:APP_REDRAW_NONE;
}
static bool category_storage_ready(void) {
    board_sd_info_t sd={0}; board_sd_get_info(&sd);
    const char *identity=sd.mounted?sd.identity:"builtin";
    if(usb_storage_active() || strcmp(identity,s_cache_library) || (sd.present&&!sd.mounted)) {
        copy_text(s_category_ui.notice,sizeof(s_category_ui.notice),"书库已变化，请退出并重新打开书架");return false;
    }
    if(!s_collection.ready || s_collection.blocked || !book_collection_migrated(&s_collection) ||
       !book_shelf_categories_logical(&s_shelf_categories) || s_collection_bound_generation!=s_collection.generation ||
       book_category_pending(app_settings_books_dir())) {
        copy_text(s_category_ui.notice,sizeof(s_category_ui.notice),"分类资料不可用，请先重试读取");return false;
    }
    return true;
}
static const char *category_error(esp_err_t err) {
    if(err==ESP_ERR_INVALID_ARG)return "名称须为1至24字且不能使用保留名称";
    if(err==ESP_ERR_INVALID_STATE)return "已有同名分类，或分类资料暂不可写";
    if(err==ESP_ERR_INVALID_SIZE)return "分类或书目已达到保存上限";
    if(err==ESP_ERR_NO_MEM)return "内存不足，未保存，请稍后重试";
    return "保存失败，原归属与勾选已保留";
}
static app_redraw_t category_tick(app_ctx_t *ctx) {
    if(s_category_ui.command==CATEGORY_IDLE) {
        if(ctx->consumed)return APP_REDRAW_NONE;
        if(s_category_ui.screen!=CATEGORY_EDIT)return category_prepare_covers(ctx)?APP_REDRAW_PAGE:APP_REDRAW_NONE;
        bool held=!ctx->released && ctx->touch && ctx->touch->touched && ctx->touch->count==1;
        if(ui_keyboard_hold_tick(held,ctx->touch?ctx->touch->x:0,ctx->touch?ctx->touch->y:0,ctx->now_ms)==UI_KEYBOARD_CHANGED ||
            ui_keyboard_idle_tick(ctx->touch && ctx->touch->touched,ctx->now_ms)==UI_KEYBOARD_CHANGED)return category_input_paint(ctx);
        return APP_REDRAW_NONE;
    }
    if(ctx->touch && ctx->touch->touched)return APP_REDRAW_NONE;
    book_category_command_t command=s_category_ui.command; s_category_ui.command=CATEGORY_IDLE;
    if(command==CATEGORY_SEARCH){search_begin();return APP_REDRAW_PAGE;}
    if(command==CATEGORY_OPEN) {
        uint32_t id=s_category_ui.destination; category_reset(); s_view=SHELF; s_shelf_selecting=false; clear_selection(); category_select(ctx,id);return APP_REDRAW_PAGE;
    }
    if(command==CATEGORY_RECOVER) {
        if (legacy_category_recover()) {
            (void)shelf_collection_load(true);
            scan_shelf(ctx);
            s_category_ui.books=s_shelf; s_category_ui.count=s_count;
        }
        s_category_ui.blocked=!book_shelf_categories_logical(&s_shelf_categories) || s_collection_bound_generation!=s_collection.generation || !s_collection.ready || s_collection.blocked ||
            book_category_pending(app_settings_books_dir());
        copy_text(s_category_ui.notice,sizeof(s_category_ui.notice),s_category_ui.blocked?"分类资料仍不可用，未覆盖原资料":"分类资料已重新读取");
        return APP_REDRAW_PAGE;
    }
    if(!category_storage_ready())return APP_REDRAW_PAGE;
    if(command==CATEGORY_ADD){category_start_books(ctx,s_category_ui.destination,false);return APP_REDRAW_PAGE;}
    esp_err_t err=ESP_OK; unsigned changed=0; uint32_t created=0,target=s_category_ui.destination;
    bool create_assign=command==CATEGORY_CREATE && s_category_ui.picker;
    bool adding=s_category_ui.screen==CATEGORY_BOOKS && !s_category_ui.selecting_only;
    if(command==CATEGORY_CREATE)err=book_collection_create(&s_collection,s_category_ui.draft,
        create_assign?s_shelf:NULL,create_assign?s_count:0,true,&created,&changed);
    else if(command==CATEGORY_MOVE)err=book_collection_assign(&s_collection,s_shelf,s_count,true,target,&changed);
    else if(command==CATEGORY_UNCLASSIFY)err=book_collection_unclassify(&s_collection,s_shelf,s_count,s_category_ui.source,&changed);
    else if(command==CATEGORY_RENAME)err=book_collection_rename(&s_collection,s_category_ui.source,s_category_ui.draft);
    else if(command==CATEGORY_REMOVE)err=book_collection_remove(&s_collection,s_category_ui.source);
    else if(command==CATEGORY_UP || command==CATEGORY_DOWN)err=book_collection_order(&s_collection,s_category_ui.source,command==CATEGORY_UP?-1:1);
    else if(command==CATEGORY_UNDO)err=book_collection_undo(&s_collection);
    if(err!=ESP_OK){copy_text(s_category_ui.notice,sizeof(s_category_ui.notice),category_error(err));return APP_REDRAW_PAGE;}
    if(s_category_ui.screen==CATEGORY_EDIT)ui_keyboard_end();
    book_collection_apply(&s_collection,s_shelf,s_count);
    bool bound=shelf_collection_sync(false); shelf_categories_recount();
    s_category_ui.can_undo=book_collection_can_undo(&s_collection);
    if(!bound){
        s_category_ui.blocked=true;s_category_ui.screen=CATEGORY_LIST;
        copy_text(s_category_ui.notice,sizeof(s_category_ui.notice),"已保存，内存不足，请重试读取");
        copy_text(s_category_result,sizeof(s_category_result),s_category_ui.notice);
        sort_shelf(ctx);return APP_REDRAW_PAGE;
    }
    if(command==CATEGORY_CREATE && !create_assign) {
        category_start_books(ctx,created,false); return APP_REDRAW_PAGE;
    }
    if(command==CATEGORY_MOVE || command==CATEGORY_UNCLASSIFY || create_assign) {
        if (changed) snprintf(s_category_result,sizeof(s_category_result),"%s %u 本",command==CATEGORY_UNCLASSIFY?"已移出本类":"已归类",changed);
        else copy_text(s_category_result,sizeof(s_category_result),"无需变更，原归属不变");
        for (int i=0;i<s_count;++i) if (s_shelf[i].selected) s_shelf[i].new_book=false;
        book_view_t parent=s_category_parent;
        category_reset(); s_view=parent==BULK?BULK:SHELF; s_shelf_selecting=false; clear_selection();
        if(adding || create_assign)category_select(ctx,create_assign?created:target);
        else {sort_shelf(ctx);if(s_view==SHELF)shelf_category_restore(ctx);}
        if(s_view==BULK)copy_text(s_batch_message,sizeof(s_batch_message),s_category_result);
    } else if (command==CATEGORY_UNDO && s_category_quick) {
        category_reset(); s_view=SHELF;
        copy_text(s_category_result,sizeof(s_category_result),"已撤销上次操作");
        sort_shelf(ctx); shelf_category_restore(ctx);
    } else {
        s_category_ui.screen=(command==CATEGORY_UP || command==CATEGORY_DOWN)?CATEGORY_ORDER:CATEGORY_LIST;
        copy_text(s_category_ui.notice,sizeof(s_category_ui.notice),command==CATEGORY_RENAME?"分类名称已保存":
            command==CATEGORY_REMOVE?"分类已删除，图书已变为未分类":command==CATEGORY_UNDO?"已撤销上次操作":"分类顺序已保存");
        copy_text(s_category_result,sizeof(s_category_result),s_category_ui.notice);
        book_category_ui_refresh(&s_category_ui,&s_shelf_categories,s_category_ui.source);
        sort_shelf(ctx);
    }
    return APP_REDRAW_PAGE;
}

static void refresh_cached_progress(app_ctx_t* ctx) {
    const uint32_t revision=book_progress_revision();
    if(s_progress_revision==revision){if(s_view==SHELF)shelf_category_restore(ctx);return;}
    s_progress_revision=revision;
    // 目录没变也要补一次：阅读只写进度，不改存储版本，否则刚读完的书不会出现。
    // Backfill even when the listing is unchanged: reading writes progress and never bumps the
    // store revision, so a freshly read book would otherwise stay invisible until a manual rescan.
    const bool added = shelf_backfill_read_books(NULL);
    bool recent_changed = false;
    char latest[BOOK_STORE_PATH_MAX];
    if (book_progress_last_path(latest, sizeof(latest))) {
        for (int i = 0; i < s_count; ++i) {
            shelf_entry_t *item = &s_shelf[i];
            if (strcmp(item->path, latest) || item->removed) continue;
            book_progress_t progress;
            item->has_progress = book_progress_load(item->path, item->size, &progress);
            uint32_t next_recent=item->has_progress?progress.last_open_s:0;
            recent_changed=item->recent!=next_recent;
            item->pct = item->has_progress ? progress.pct : 0;
            item->chapter = item->has_progress ? progress.chapter : 0;
            item->recent = next_recent;
            copy_text(s_latest_path, sizeof(s_latest_path), latest);
            break;
        }
    }
    // 新增条目必须重排才能重算可见列表；只刷新进度时沿用原有顺序。
    // Added entries need a re-sort to recompute the visible list; a progress refresh alone
    // keeps the current order.
    if (added || (shelf_query().recent_sort && recent_changed)) { (void)shelf_collection_sync(added); shelf_categories_recount(); sort_shelf(ctx); }
    if (s_view==SHELF) shelf_category_restore(ctx);
}
static void return_to_cached_shelf(app_ctx_t* ctx) {
    if (s_shelf_cache_valid && s_store_revision == book_store_revision()) refresh_cached_progress(ctx);
    else scan_shelf(ctx);
}

static void refresh_capacity(void) {
    book_store_root_t roots[BOOK_STORE_ROOT_MAX];
    int count = 0;
    s_storage[0] = 0;
    book_store_roots(roots, &count);
    for (int i = 0; i < count; ++i) {
        if (i && !roots[i].is_flash && !roots[i - 1].is_flash) continue;
        char line[64];
        snprintf(line, sizeof(line), "%s%s %.1f MB", s_storage[0] ? " · " : "", roots[i].is_flash ? "内置余" : "TF余", book_store_free_bytes(&roots[i]) / 1048576.0);
        strncat(s_storage, line, sizeof(s_storage) - strlen(s_storage) - 1);
    }
}
static book_shelf_management_t shelf_management_context(void) {
    return (book_shelf_management_t){
        .retries = &s_delete_retries, .current_path = s_reader_session.book.path,
        .store_revision = &s_store_revision, .close = free_book, .discard_progress = pending_discard,
    };
}
static void shelf_managed_removed(void *ctx, const char *path) {
    (void)ctx;
    for (int i = 0; i < s_count; ++i)
        if (!strcmp(s_shelf[i].path, path)) s_shelf[i].removed = true;
    refresh_capacity();
}
static void manage_apply(app_ctx_t* ctx) {
    book_shelf_management_t management = shelf_management_context();
    book_shelf_manage_result_t result = book_shelf_manage_one(&management, &s_managed,
        s_delete_confirm, &s_file_removed, shelf_managed_removed, NULL);
    if (result.reserve_failed) {
        copy_text(s_manage_message, sizeof(s_manage_message), "内存不足，无法保留删除重试记录");
        s_clear_confirm = false;
        return;
    }
    esp_err_t err = result.error;
    s_clear_confirm = false;
    if (err != ESP_OK) {
        copy_text(s_manage_message, sizeof(s_manage_message), s_file_removed ?
                  "文件已删除，进度清理失败，请重试" : s_delete_confirm ?
                  "文件删除失败，请检查存储后重试" : "进度清理失败，请重新尝试");
        return;
    }
    s_view = SHELF;
    int write = 0, leaf = ctx->leaf;
    for (int i = 0; i < s_count; ++i) {
        if (!strcmp(s_shelf[i].path, s_managed.path)) {
            if (s_file_removed) continue;
            s_shelf[i].has_progress = false; s_shelf[i].pct = 0; s_shelf[i].chapter = 0; s_shelf[i].recent = 0;
        }
        s_shelf[write++] = s_shelf[i];
    }
    s_count = write;
    sort_shelf(ctx);
    ctx->leaf = leaf < leaves() ? leaf : leaves() - 1;
}

/* ---- 输入与生命周期 / Input and lifecycle ---- */

static bool reader_turn_chrome(app_ctx_t *ctx) {
    // 翻页只收起面板与滑块，全屏状态保持不变；全屏切换走工具栏按钮与按键。
    // Turns collapse panels and sliders only; full screen stays untouched and is
    // toggled from the toolbar button or keys.
    (void)ctx;
    bool changed = s_reader_panel != READER_PANEL_NONE || s_reader_slider >= 0;
    s_reader_panel = READER_PANEL_NONE;
    s_reader_slider = -1;
    if (changed) { invalidate_prep(); s_reader_text_frame = false; }
    return changed;
}
static void reader_turn_save(void *context) {
    (void)context;
    save_progress();
}

static bool reader_turn_load(void *context, size_t chapter, bool last) {
    return load_chapter(context, chapter, 0, last);
}

static app_redraw_t turn_page(app_ctx_t* ctx, int dir) {
#if CONFIG_READER_PERF
    reader_perf_begin("turn");
    READER_PERF_SCOPE(READER_PERF_ACTION);
#endif
    // 翻页就是用户操作：音量键、上键和蓝牙翻页器都不经过触摸帧，只有在这里登记才能重置自动休眠计时。
    // A turn is user activity: the side keys, the up key and the page turner never pass a touch frame,
    // so only noting it here restarts the idle-lock clock.
    ctx->user_activity = true;
    TD_LOGI("TURN dir=%d ch=%u pg=%u", dir, (unsigned)s_reader_session.position.chapter, (unsigned)s_reader_session.position.page);
    s_reader_footer_strong = false;
    if (!s_text || s_clear_confirm) return APP_REDRAW_NONE;
    const size_t prior_chapter = s_reader_session.position.chapter;
    // 页面图片表可能随跨章加载替换，旧页资格以实际成功显示的画面为准。
    // Chapter loads may replace the image table; the old-page eligibility comes from the displayed frame.
    bool from_text_frame = s_reader_text_frame;
    bool from_stable_frame = s_reader_stable_frame;
    const reader_turn_ops_t turn_ops = {
        .context = ctx, .save = reader_turn_save, .load = reader_turn_load,
    };
    reader_turn_result_t moved = reader_turn_move(&s_reader_session.position, dir,
        book_layout_page_count(), book_chapter_count(), &turn_ops);
    if (moved == READER_TURN_EDGE) return reader_turn_chrome(ctx) ? APP_REDRAW_PAGE : APP_REDRAW_NONE;
    if (moved == READER_TURN_LOAD_FAILED) { set_reader_view(s_text ? TOC : SHELF); ctx->leaf = s_text ? s_reader_session.position.chapter / BOOK_TOC_ROWS : 0; return APP_REDRAW_PAGE; }
    skip_hidden_image_pages(ctx, dir);
    if (!s_text) { s_view = SHELF; return APP_REDRAW_PAGE; }
    // 先选择旧布局的目标页，再切全屏，保留目标位置而不是跨过未读段落。
    // Choose the destination in the old layout first, then reflow to retain its reading position.
    bool chrome_changed = reader_turn_chrome(ctx);
    if (!s_text) return APP_REDRAW_PAGE;
    s_jump_offset = SIZE_MAX;
    s_last_turn_ms = s_stats_activity_ms = ctx->now_ms;
    ++s_stats_pending_turns;
    ++s_session_turns;
    app_reading_journal_record(s_reader_session.book.path,s_reader_session.book.title,
        s_reader_session.book.file_size,(unsigned)s_reader_session.position.chapter,0,1);
    uint8_t full_pages = app_settings_reader_full_pages();
    if (full_pages) ++s_turns;
    else s_turns = 0;
    s_reader_cleanup = full_pages > 0 && s_turns >= full_pages;
    if (s_reader_cleanup) s_turns = 0;
    if (s_unsaved < 8) ++s_unsaved;
    if (s_unsaved >= 8 && !s_save_failed) TD_TIME("save_progress", save_progress());
    ESP_LOGI(TAG, "turn chapter=%u page=%u/%u pct=%u", (unsigned)s_reader_session.position.chapter, (unsigned)s_reader_session.position.page + 1, (unsigned)book_layout_page_count(), percent(s_reader_session.position.page));
    // 预渲染未命中时也使用 GL16；到达设定页数后在本次翻页整屏全刷。
    // A cache miss still uses GL16; the selected turn count triggers a full-screen refresh on this turn.
    app_redraw_t redraw = paint_reading(ctx, MODE_GL16);
    const reader_turn_frame_t frame = {
        .chrome_changed = chrome_changed, .area_redraw = redraw == APP_REDRAW_AREA,
        .from_text = from_text_frame, .from_stable = from_stable_frame,
        .images = book_layout_page_image_count(s_reader_session.position.page) != 0,
        .background = app_settings_book_background_path()[0] != 0,
        .cleanup = s_reader_cleanup, .water_effect = app_settings_reader_turn_effect() == 1,
        .chapter_changed = s_reader_session.position.chapter != prior_chapter,
    };
    reader_turn_effects_t effects = reader_turn_effects(&frame);
    s_reader_turn_pending = effects.text;
    // 水波纹走原厂 GL16 表，灰阶转换逐对校准，阅读背景页同样适用；插图页仍排除。
    // The ripple rides the vendor GL16 table with per-pair calibrated transitions, so
    // reading backgrounds qualify; image pages stay excluded.
    s_water_turn_pending = effects.water;
    s_water_turn_dir = dir > 0 ? E0470_TURN_RTL : E0470_TURN_LTR;
    // 换章页脚恢复默认 GL16 清理；普通页脚仍走短 DU，整屏清理不重复输出页脚。
    // Restore default GL16 cleanup for chapter footers; ordinary footers use short DU and full cleanup never replays them.
    s_reader_footer_strong = effects.footer_strong;
    return chrome_changed ? APP_REDRAW_PAGE : redraw;
}
static void reader_reflow_begin(void) {
    lock_draw();
    invalidate_prep();
}

static bool reader_reflow_build(void) {
    return reader_session_build_layout(&s_reader_session, reader_session_content(&s_reader_session), body_rect(), s_px);
}

static const reader_reflow_t *reader_reflow_context(void) {
    _Static_assert((int)MARGIN_LEFT == READER_REFLOW_LEFT && (int)MARGIN_RIGHT == READER_REFLOW_RIGHT &&
                   (int)MARGIN_TOP == READER_REFLOW_TOP && (int)MARGIN_BOTTOM == READER_REFLOW_BOTTOM &&
                   (int)MARGIN_COUNT == READER_REFLOW_MARGINS && BOOK_MARGIN_MIN == READER_REFLOW_MARGIN_MIN &&
                   BOOK_MARGIN_MAX == READER_REFLOW_MARGIN_MAX, "reader margin contract");
    static const reader_reflow_t reflow = {
        .position = &s_reader_session.position, .px = &s_px,
        .margins = s_margin, .line_spacing = &s_line_spacing,
        .begin = reader_reflow_begin, .end = unlock_draw,
        .refresh_lead = refresh_chapter_lead, .build = reader_reflow_build, .save = save_progress,
    };
    return &reflow;
}

static app_redraw_t apply_reader_layout(app_ctx_t* ctx, int next_px,
                                        const int next_margin[MARGIN_COUNT],
                                        uint8_t next_line, uint8_t next_para) {
#if CONFIG_READER_PERF
    reader_perf_begin("reflow");
    READER_PERF_SCOPE(READER_PERF_ACTION);
#endif
    (void)ctx;
    return reader_reflow_layout(reader_reflow_context(), next_px, next_margin, next_line, next_para)
        ? APP_REDRAW_PAGE : APP_REDRAW_NONE;
}

static app_redraw_t apply_reader_typography(app_ctx_t* ctx, int tracking_index) {
    (void)ctx;
    if (!s_text) return APP_REDRAW_NONE;
    return reader_reflow_typography(reader_reflow_context(), tracking_index)
        ? APP_REDRAW_PAGE : APP_REDRAW_NONE;
}

static app_redraw_t apply_reader_indent(app_ctx_t* ctx, int em, int adjustment) {
    (void)ctx;
    if (!s_text) return APP_REDRAW_NONE;
    return reader_reflow_indent(reader_reflow_context(), em, adjustment)
        ? APP_REDRAW_PAGE : APP_REDRAW_NONE;
}

static app_redraw_t reader_indent_finish(app_ctx_t *ctx,bool commit) {
    int value=s_indent_hold.value;
    bool valid=commit&&!s_indent_hold.cancelled;
    reader_indent_hold_cancel();
    if(valid){
        app_redraw_t redraw=apply_reader_indent(ctx,app_settings_book_indent(),value);
        if(redraw!=APP_REDRAW_NONE)return redraw;
    }
    return reader_indent_feedback(ctx);
}
static app_redraw_t reader_indent_gesture(app_ctx_t *ctx,const ui_gesture_event_t *ev,bool *handled) {
    *handled=false;
    if(s_view!=READING||s_reader_panel!=READER_PANEL_LAYOUT_SETTINGS||s_clear_confirm){reader_indent_hold_cancel();return APP_REDRAW_NONE;}
    if(ev->type==UI_GESTURE_PRESS){
        reader_indent_hold_cancel();int control=indent_adjust_hit(ev->x0,ev->y0);
        if(control<0||ctx->consumed||ctx->now_ms<0)return APP_REDRAW_NONE;
        s_indent_hold.active=true;s_indent_hold.control=control;s_indent_hold.value=app_settings_book_indent_adjust();
        s_indent_hold.last_ms=ctx->now_ms;s_indent_hold.next_ms=ctx->now_ms+UI_LONG_PRESS_MS;
        s_pressed_control=-1;*handled=true;
        return reader_indent_feedback(ctx);
    }
    if(!s_indent_hold.active)return APP_REDRAW_NONE;
    *handled=true;
    bool inside=indent_adjust_hit(ev->x,ev->y)==s_indent_hold.control;
    if(ctx->consumed||!inside||ctx->now_ms<s_indent_hold.last_ms){
        s_indent_hold.cancelled=true;
        if(ev->type==UI_GESTURE_TAP||ev->type==UI_GESTURE_CANCEL||ctx->released)return reader_indent_finish(ctx,false);
        return reader_indent_feedback(ctx);
    }
    if(ev->type==UI_GESTURE_TAP){
        if(!s_indent_hold.repeated){
            int next=s_indent_hold.control==1?0:s_indent_hold.value+(s_indent_hold.control==0?-1:1);
            s_indent_hold.value=next<-100?-100:next>100?100:next;
        }
        return reader_indent_finish(ctx,true);
    }
    if(ev->type==UI_GESTURE_CANCEL){
        bool released=ctx->released&&ctx->touch&&!ctx->touch->touched;
        return reader_indent_finish(ctx,released);
    }
    if(ev->type==UI_GESTURE_SWIPE_L||ev->type==UI_GESTURE_SWIPE_R||ev->type==UI_GESTURE_SWIPE_U||ev->type==UI_GESTURE_SWIPE_D)return reader_indent_finish(ctx,false);
    return APP_REDRAW_NONE;
}
static app_redraw_t reader_indent_hold_tick(app_ctx_t *ctx) {
    if(!s_indent_hold.active)return APP_REDRAW_NONE;
    if(s_view!=READING||s_reader_panel!=READER_PANEL_LAYOUT_SETTINGS||s_clear_confirm||ctx->request_app||ctx->request_menu||ctx->request_return){reader_indent_hold_cancel();return APP_REDRAW_NONE;}
    if(s_indent_hold.cancelled)return APP_REDRAW_NONE;
    bool held=!ctx->released&&ctx->touch&&ctx->touch->touched&&ctx->touch->count==1;
    if(!held||ctx->now_ms<s_indent_hold.last_ms||indent_adjust_hit(ctx->touch->x,ctx->touch->y)!=s_indent_hold.control){
        s_indent_hold.cancelled=true;return reader_indent_feedback(ctx);
    }
    if(ctx->consumed)return APP_REDRAW_NONE;
    s_indent_hold.last_ms=ctx->now_ms;
    if(s_indent_hold.control==1||ctx->now_ms<s_indent_hold.next_ms)return APP_REDRAW_NONE;
    // 推屏慢时不补发跳变，每次仍只改1px；重排和NVS写入留到松手。
    // Never catch up after slow output: retain 1px steps and defer reflow/NVS writes until release.
    s_indent_hold.next_ms=ctx->now_ms+120;s_indent_hold.repeated=true;
    int next=s_indent_hold.value+(s_indent_hold.control==0?-1:1);
    if(next<-100||next>100)return APP_REDRAW_NONE;
    s_indent_hold.value=next;ctx->user_activity=true;
    return reader_indent_feedback(ctx);
}

static app_redraw_t reader_manual_refresh(app_ctx_t* ctx) {
    (void)ctx;
    if (!s_text) return APP_REDRAW_NONE;
    s_reader_panel = READER_PANEL_NONE;
    s_reader_slider = -1;
    s_reader_footer_pending = s_reader_image_refresh_pending = false;
    s_reader_footer_strong = false;
    s_water_turn_pending = s_reader_cleanup = false;
    s_du_count = s_turns = 0;
    invalidate_prep();
    return APP_REDRAW_FULL;
}

static app_redraw_t apply_reader_option(app_ctx_t* ctx, int kind) {
    (void)ctx;
    if (!s_text || kind < 0 || kind > 2) return APP_REDRAW_NONE;
    bool previous = kind == 0 ? app_settings_reader_statusbar() :
                    kind == 1 ? app_settings_reader_hide_images() : app_settings_book_vertical();
    // 重新开启插图时，先补齐尺寸；文件读取必须在绘制锁外。
    // Probe missing dimensions before re-enabling images, outside the drawing lock.
    if (kind == 1 && previous) for (size_t i = 0; i < s_block_count; ++i) {
        blk_t *block = &s_blocks[i];
        if (block->image < 0 || (size_t)block->image >= s_image_count ||
            (block->image_width && block->image_height)) continue;
        unsigned width = 0, height = 0;
        if (book_chapter_image_dimensions(s_reader_session.position.chapter, s_images[block->image], &width, &height) == ESP_OK) {
            block->image_width = width; block->image_height = height;
        }
        vTaskDelay(1);
    }
    lock_draw();
    size_t off = book_layout_page_start_offset(s_reader_session.position.page);
    invalidate_prep();
    bool ok = reader_reflow_option_locked(reader_reflow_context(), kind, previous, off);
    if (!ok) {
        copy_text(s_reader_notice, sizeof(s_reader_notice), "排版失败，请重试");
        s_reader_notice_until = esp_timer_get_time() / 1000 + 1800;
    }
    unlock_draw();
    if (ok) {
        release_page_images();
        prepare_inline_image();
        s_jump_offset = SIZE_MAX;
        save_progress();
    }
    s_reader_footer_pending = s_reader_image_refresh_pending = false;
    s_reader_footer_strong = false;
    s_water_turn_pending = false;
    return APP_REDRAW_PAGE;
}

// 返回主页以本次开书来源为准；分类页码已在书架开书前记忆，不被目录/工具栏覆盖。
// Return to this opening's origin; shelf category/page were saved before opening, independently of TOC/tools.
static app_redraw_t reader_return(app_ctx_t *ctx) {
    save_progress();
    if (s_reader_return_home) {
        ui_nav_request(ctx, 0);
        return APP_REDRAW_NONE;
    }
    free_book();
    s_view = SHELF;
    s_reader_panel = READER_PANEL_NONE;
    return_to_cached_shelf(ctx);
    return APP_REDRAW_PAGE;
}

static app_redraw_t toggle_reader_fullscreen(app_ctx_t* ctx) {
    (void)ctx;
    if (!s_text) return APP_REDRAW_NONE;
    reader_reflow_result_t result = reader_reflow_fullscreen(reader_reflow_context(), &s_reader_fullscreen);
    if (result != READER_REFLOW_APPLIED) {
        if (result == READER_REFLOW_LOST_LAYOUT) {
            free_book();
            s_view = SHELF;
            copy_text(s_message, sizeof(s_message), "排版失败，请重新扫描并打开图书");
            return APP_REDRAW_PAGE;
        }
        return APP_REDRAW_NONE;
    }
    release_page_images();
    prepare_inline_image();
    s_jump_offset = SIZE_MAX;
    s_reader_panel = READER_PANEL_NONE;
    // 切换成功即落盘偏好；重排失败的回滚路径不写。/ Persist the preference on a successful
    // toggle only; the failed-reflow rollback path never writes.
    app_settings_set_reader_open_fullscreen(s_reader_fullscreen);
    // 书架点击直接调 open_book，只读这个待开值而不重新读设置；不同步的话关书再进会回到切换前的模式。
    // A shelf tap calls open_book directly and reads this pending value without re-reading the
    // setting, so without this sync closing and reopening the book restores the pre-toggle mode.
    s_requested_fullscreen = s_reader_fullscreen;
    save_progress();
    return APP_REDRAW_PAGE;
}

static app_redraw_t select_reading_font_item(app_ctx_t* ctx, const ttf_font_item_t* item) {
    if (!item || !strcmp(ttf_font_path(), item->path)) return APP_REDRAW_NONE;
    char previous[TTF_FONT_PATH_MAX];
    copy_text(previous, sizeof(previous), ttf_font_path());
    if (ttf_font_open(item->path) != ESP_OK) return APP_REDRAW_NONE;
    size_t off = book_layout_page_start_offset(s_reader_session.position.page);
    lock_draw();
    invalidate_prep();
    bool ok = reader_session_build_layout(&s_reader_session, reader_session_content(&s_reader_session), body_rect(), s_px);
    if (ok) s_reader_session.position.page = book_layout_page_for_offset(off);
    else if (previous[0]) {
        (void)ttf_font_open(previous);
        (void)reader_session_build_layout(&s_reader_session, reader_session_content(&s_reader_session), body_rect(), s_px);
    }
    unlock_draw();
    if (!ok) return APP_REDRAW_NONE;
    app_settings_set_font_path(item->path);
    copy_text(s_font_path, sizeof(s_font_path), item->path);
    save_progress();
    return APP_REDRAW_PAGE;
}

// 高度先在当前阅读状态试排，保存成功后才接受；失败不把候选高度留在设置里。
// Trial the height in reader state before persistence; failed candidates never remain in settings.
static app_redraw_t apply_reader_chrome_heights(int header, int footer, bool reset) {
    const int previous[2] = {s_chrome_height[0], s_chrome_height[1]};
    const bool reflow = !s_reader_fullscreen && (header != previous[0] || footer != previous[1]);
    size_t offset = book_layout_page_start_offset(s_reader_session.position.page);
    lock_draw();
    invalidate_prep();
    s_chrome_height[0] = header;
    s_chrome_height[1] = footer;
    bool ok = true, restored = true;
    if (reflow) {
        refresh_chapter_lead();
        ok = reader_reflow_build();
    }
    if (ok) ok = reset ? app_settings_set_reader_chrome_default()
        : app_settings_set_reader_chrome_heights((uint8_t)header, (uint8_t)footer);
    if (ok && reflow) s_reader_session.position.page = book_layout_page_for_offset(offset);
    if (!ok) {
        s_chrome_height[0] = previous[0];
        s_chrome_height[1] = previous[1];
        if (reflow) {
            refresh_chapter_lead();
            restored = reader_reflow_build();
        }
    }
    unlock_draw();
    if (!restored) {
        free_book();
        s_view = SHELF;
        copy_text(s_message, sizeof(s_message), "排版失败，请重新扫描并打开图书");
    } else if (!ok) copy_text(s_chrome_error, sizeof(s_chrome_error), "调整失败，请重试");
    else {
        s_chrome_error[0] = 0;
        if (reflow) {
            release_page_images();
            prepare_inline_image();
            s_jump_offset = SIZE_MAX;
            save_progress();
        }
    }
    s_reader_footer_pending = s_reader_image_refresh_pending = false;
    s_reader_footer_strong = s_water_turn_pending = false;
    return APP_REDRAW_PAGE;
}

static int slider_index(EpdRect rect, int x, int count) {
    int span = rect.width - 128;
    int pos = x - rect.x - 64;
    int index = span > 0 && count > 1 ? (pos * (count - 1) + span / 2) / span : 0;
    if (index < 0) index = 0;
    if (index >= count) index = count - 1;
    return index;
}
static int reader_margin_input(EpdRect rect, int x, int current) {
    int level = reader_margin_level(current);
    if (x < rect.x + 64) --level;
    else if (x >= rect.x + rect.width - 64) ++level;
    else level = slider_index(rect, x, BOOK_MARGIN_CHOICES);
    if (level == reader_margin_level(current)) return current;
    return reader_margin_value(level);
}

static EpdRect reader_slider_rect(int slider) {
    // 字号滑杆在双列字体卡片上方，绘制与拖动共用位置。/ The size rail sits above the paired cards; paint and drag share its position.
    // 1..4 是四向边距的 2×2 块，5/6 是行距与段距，7 是字间距。
    // Rails 1..4 form the 2x2 margin block, 5/6 are line and paragraph spacing, 7 is tracking.
    if (slider == 0) return (EpdRect){36,700-reader_ui_extra(112),612,76};
    if (slider == 1) return (EpdRect){36, 540, 294, 66};
    if (slider == 2) return (EpdRect){354, 540, 294, 66};
    if (slider == 3) return (EpdRect){36, 624, 294, 66};
    if (slider == 4) return (EpdRect){354, 624, 294, 66};
    if (slider == 5) return (EpdRect){36, 708, 294, 66};
    if (slider == 6) return (EpdRect){354, 708, 294, 66};
    return (EpdRect){36, 792, 612, 66};
}

static int reader_slider_at(uint16_t x, uint16_t y) {
    int first = s_reader_panel == READER_PANEL_FONT_SETTINGS ? 0 : 1;
    int end = s_reader_panel == READER_PANEL_FONT_SETTINGS ? 1 : 8;
    for (int i = first; i < end; ++i) if (ui_rect_hit(reader_slider_rect(i), x, y)) return i;
    return -1;
}

static void reader_slider_map(int slider, int x) {
    if (slider == 0) s_reader_preview_px = BOOK_PX_MIN + slider_index(reader_slider_rect(0), x, BOOK_PX_MAX - BOOK_PX_MIN + 1);
    else if (slider >= 1 && slider <= 4)
        s_reader_preview_margin[slider - 1] =
            reader_margin_value(slider_index(reader_slider_rect(slider), x, BOOK_MARGIN_CHOICES));
    else if (slider == 5) s_reader_preview_line = 110 + slider_index(reader_slider_rect(5), x, 41);
    else if (slider == 6) s_reader_preview_para = app_settings_book_paragraph_value((uint8_t)slider_index(reader_slider_rect(6), x, APP_BOOK_PARAGRAPH_COUNT));
    else if (slider == 7) s_reader_preview_tracking = slider_index(reader_slider_rect(7), x, APP_BOOK_TRACK_COUNT);
}

static void reader_slider_begin(int slider, int x) {
    s_reader_slider = slider;
    s_reader_preview_px = s_px;
    memcpy(s_reader_preview_margin, s_margin, sizeof(s_reader_preview_margin));
    s_reader_preview_line = app_settings_book_line_spacing();
    s_reader_preview_para = app_settings_book_paragraph_spacing();
    s_reader_preview_tracking = app_settings_book_tracking();
    EpdRect rect = reader_slider_rect(slider);
    s_reader_slider_endpoint = x < rect.x + 64 || x >= rect.x + rect.width - 64;
    if (slider >= 1 && slider <= 4) {
        int *value = &s_reader_preview_margin[slider - 1];
        *value = reader_margin_input(rect, x, *value);
        return;
    }
    if (slider == 6) {
        int level = app_settings_book_paragraph_level((uint8_t)s_reader_preview_para);
        if (x < rect.x + 64) --level;
        else if (x >= rect.x + rect.width - 64) ++level;
        else level = slider_index(rect, x, APP_BOOK_PARAGRAPH_COUNT);
        if (level < 0) level = 0;
        if (level >= APP_BOOK_PARAGRAPH_COUNT) level = APP_BOOK_PARAGRAPH_COUNT - 1;
        s_reader_preview_para = app_settings_book_paragraph_value((uint8_t)level);
        return;
    }
    int *value = slider == 0 ? &s_reader_preview_px : slider == 5 ? &s_reader_preview_line :
                 &s_reader_preview_tracking;
    int min = slider == 0 ? BOOK_PX_MIN : slider == 5 ? 110 : 0;
    int max = slider == 0 ? BOOK_PX_MAX : slider == 5 ? 150 : APP_BOOK_TRACK_COUNT - 1;
    if (x < rect.x + 64) --*value;
    else if (x >= rect.x + rect.width - 64) ++*value;
    else reader_slider_map(slider, x);
    if (*value < min) *value = min;
    if (*value > max) *value = max;
}

static void reader_slider_move(int x) {
    int slider = s_reader_slider;
    if (slider < 0) return;
    s_reader_slider_endpoint = false;
    reader_slider_map(slider, x);
}

static app_redraw_t reader_slider_commit(app_ctx_t* ctx) {
    if (s_reader_slider < 0) return APP_REDRAW_NONE;
    int slider = s_reader_slider;
    int px = s_reader_preview_px, line = s_reader_preview_line;
    int para = s_reader_preview_para;
    int tracking = s_reader_preview_tracking;
    s_reader_slider = -1;
    if (slider == 7) return apply_reader_typography(ctx, tracking);
    return apply_reader_layout(ctx, px, s_reader_preview_margin, line, (uint8_t)para);
}

static app_redraw_t reading_settings_action(app_ctx_t *ctx, uint16_t x, uint16_t y) {
    if (y < reading_settings_top() || ui_rect_hit((EpdRect){36, 1141, 612, 58}, x, y)) {
        s_reader_panel = READER_PANEL_NONE;
    } else if (ui_rect_hit(reading_setting_rect(0), x, y)) {
        static const uint8_t options[] = {5, 10, 15, 30, 0};
        unsigned next = 0;
        for (unsigned i = 0; i < 5; ++i)
            if (app_settings_reader_full_pages() == options[i]) { next = (i + 1) % 5; break; }
        app_settings_set_reader_full_pages(options[next]);
        s_turns = 0;
    } else if (ui_rect_hit(reading_setting_rect(1), x, y)) {
        app_settings_set_reader_turn_effect(!app_settings_reader_turn_effect());
    } else if (ui_rect_hit(reading_setting_rect(2), x, y)) {
        s_reader_panel = READER_PANEL_TURN_SETTINGS;
    } else if (ui_rect_hit(reading_setting_rect(3), x, y)) {
        s_reader_panel = READER_PANEL_KEY_SETTINGS;
        s_key_scroll = 0;
        s_key_drag = false;
    } else if (ui_rect_hit(reading_setting_rect(7), x, y)) {
        s_reader_panel = READER_PANEL_CHROME_SETTINGS;
        s_chrome_scroll = 0;
        s_chrome_drag = false;
        s_chrome_error[0] = 0;
    } else {
        for (int i = 0; i < 3; ++i)
            if (ui_rect_hit(reading_setting_rect(4 + i), x, y)) return apply_reader_option(ctx, i);
        return APP_REDRAW_NONE;
    }
    invalidate_prep();
    return APP_REDRAW_PAGE;
}

static app_redraw_t reader_panel_action(app_ctx_t* ctx, uint16_t x, uint16_t y) {
    // 注释弹层没有按钮：点任意处收起，回到刚才那一页。
    // The note sheet has no buttons: any tap dismisses it and returns to the same page.
    if (s_reader_panel == READER_PANEL_NOTE) {
        s_reader_panel = READER_PANEL_NONE;
        invalidate_prep();
        return APP_REDRAW_PAGE;
    }
    if (s_reader_panel == READER_PANEL_TOOLS) {
        for (int i = 0; i < BOOK_TOOL_COUNT; ++i) if (ui_rect_hit(tool_rect(i), x, y)) {
            invalidate_prep();
            if (i == 0) {
                save_progress();
                set_reader_view(TOC);
                s_message[0] = 0;
                s_toc_jump_open = s_toc_jump_drag = false;
                s_reader_panel = READER_PANEL_NONE;
                size_t toc = current_toc_position();
                ctx->leaf = toc == SIZE_MAX ? 0 : (int)(toc / BOOK_TOC_ROWS);
            } else if (i == 1) {
                (void)bookmark_toggle();
            } else if (i == 2) {
                s_reader_panel = READER_PANEL_STATS;
            } else if (i == 3) {
                s_reader_panel = READER_PANEL_REFRESH_SETTINGS;
            } else if (i == 5) {
                return toggle_reader_fullscreen(ctx);
            } else {
                s_line_spacing = app_settings_book_line_spacing();
                s_reader_slider = -1;
                s_reader_panel = READER_PANEL_FONT_SETTINGS;
            }
            return APP_REDRAW_PAGE;
        }
        s_reader_panel = READER_PANEL_NONE;
        s_reader_tools_toggle = true;
        invalidate_prep();
        return APP_REDRAW_PAGE;
    }
    if (s_reader_panel == READER_PANEL_STATS) {
        const int top = 768;
        EpdRect details = {36, top + 319, 294, 78}, recent = {354, top + 319, 294, 78};
        if (ui_rect_hit(details, x, y)) {
            s_bookmark_page = 0;
            s_bookmark_edit = s_bookmark_delete_confirm = s_bookmark_delete_error = false;
            s_bookmark_selected = 0;
            s_reader_panel = READER_PANEL_BOOKMARKS;
        }
        else if (ui_rect_hit(recent, x, y)) {
            flush_ticket_stats();
            s_recent_days_valid = book_ticket_recent_days(s_recent_days);
            s_reader_panel = READER_PANEL_STATS_RECENT;
        } else s_reader_panel = y < top ? READER_PANEL_NONE : READER_PANEL_TOOLS;
        invalidate_prep();
        return APP_REDRAW_PAGE;
    }
    if (s_reader_panel == READER_PANEL_BOOKMARKS) {
        const int top = 560;
        if (s_bookmark_delete_confirm) {
            if (y >= top + 348 && y < top + 412) {
                if (x < 342) s_bookmark_delete_confirm = s_bookmark_delete_error = false;
                else if (bookmark_delete_selected()) s_bookmark_delete_confirm = s_bookmark_delete_error = false;
                else s_bookmark_delete_error = true;
            }
        } else if (y < top) s_reader_panel = READER_PANEL_NONE;
        else if (y < top + 86) {
            if (x >= 544 && bookmark_count()) {
                s_bookmark_edit = !s_bookmark_edit;
                s_bookmark_selected = 0;
                s_bookmark_page = 0;
            } else s_reader_panel = READER_PANEL_STATS;
        } else {
            reader_bookmarks_t marks = {0};
            bool valid = bookmark_load(&marks);
            int pages = bookmark_pages(valid ? marks.count : 0);
            if (s_bookmark_edit && y >= top + 520 && y < top + 584) {
                if (x < 303) {
                    int first = s_bookmark_page * bookmark_rows();
                    for (int row = 0; row < bookmark_rows() && first + row < marks.count; ++row)
                        s_bookmark_selected |= UINT32_C(1) << (marks.count - 1 - (first + row));
                } else if (s_bookmark_selected) s_bookmark_delete_confirm = true;
            } else if (y >= top + (s_bookmark_edit ? 590 : 570)) {
                if (x < UI_LOCK_WIDTH / 2 && s_bookmark_page > 0) --s_bookmark_page;
                else if (x >= UI_LOCK_WIDTH / 2 && s_bookmark_page + 1 < pages) ++s_bookmark_page;
            } else if (valid && y >= top + 96) {
                int row = ((int)y - (top + 96)) / bookmark_row_step();
                int position = s_bookmark_page * bookmark_rows() + row;
                if (row >= 0 && row < bookmark_rows() && position < marks.count &&
                    ui_rect_hit(bookmark_row_rect(row),x,y)) {
                    int newest = (int)marks.count - 1 - position;
                    reader_bookmark_entry_t mark = marks.entries[newest];
                    if (s_bookmark_edit) s_bookmark_selected ^= UINT32_C(1) << newest;
                    else if (mark.chapter < book_chapter_count() && load_chapter(ctx, mark.chapter, mark.byte_off, false)) {
                        s_reader_panel = READER_PANEL_NONE;
                        save_progress();
                    }
                }
            }
        }
        invalidate_prep();
        return APP_REDRAW_PAGE;
    }
    if (s_reader_panel == READER_PANEL_STATS_RECENT) {
        int top = 584;
        if (y < top) s_reader_panel = READER_PANEL_NONE;
        else if (y < top + 112) s_reader_panel = READER_PANEL_STATS;
        invalidate_prep();
        return APP_REDRAW_PAGE;
    }
    if (s_reader_panel == READER_PANEL_REFRESH_SETTINGS)
        return reading_settings_action(ctx, x, y);
    if (s_reader_panel == READER_PANEL_KEY_SETTINGS || s_reader_panel == READER_PANEL_KEY_PICKER) {
        bool picker = s_reader_panel == READER_PANEL_KEY_PICKER;
        int top = picker ? 270 : KEY_SETTINGS_TOP;
        if (y < top) s_reader_panel = READER_PANEL_NONE;
        else if (ui_rect_hit((EpdRect){36, top + 29, 56, 56}, x, y) || ui_rect_hit((EpdRect){36, 1141, 612, 58}, x, y))
            s_reader_panel = picker ? READER_PANEL_KEY_SETTINGS : READER_PANEL_REFRESH_SETTINGS;
        else if (picker) {
            for (int i = 0; i < READER_KEY_ACTION_ROWS; ++i) if (ui_rect_hit(reader_key_action_card(i), x, y)) {
                app_reader_key_action_t action = s_reader_key_order[i];
                bool saved = s_reader_key_pick == 3 ? app_settings_set_reader_hold_action(action)
                    : s_reader_key_pick == 4 ? app_settings_set_reader_double_action(action)
                    : s_reader_key_pick == 5 ? app_settings_set_reader_up_action(action)
                    : app_settings_set_reader_key_action(s_reader_key_pick, action);
                if (!saved) {
                    copy_text(s_reader_key_error, sizeof(s_reader_key_error), "保存失败，请重试");
                    return APP_REDRAW_PAGE;
                }
                s_reader_panel = READER_PANEL_KEY_SETTINGS;
                return APP_REDRAW_PAGE;
            }
            return APP_REDRAW_NONE;
        } else {
            for (unsigned slot = 0; slot < 3; ++slot) if (ui_rect_hit(reader_key_card(slot), x, y)) {
                s_reader_key_pick = s_reader_key_slots[slot];
                s_reader_key_error[0] = 0;
                s_reader_panel = READER_PANEL_KEY_PICKER;
                return APP_REDRAW_PAGE;
            }
            for (int row = 0; row < KEY_ROW_COUNT; ++row) {
                if (!ui_rect_hit(reader_key_row_rect(row), x, y)) continue;
                if (row == 3) {
                    app_settings_set_reader_volume_turn(!app_settings_reader_volume_turn());
                    return APP_REDRAW_PAGE;
                }
                s_reader_key_pick = 3 + (unsigned)row;
                s_reader_key_error[0] = 0;
                s_reader_panel = READER_PANEL_KEY_PICKER;
                return APP_REDRAW_PAGE;
            }
            return APP_REDRAW_NONE;
        }
        invalidate_prep();
        return APP_REDRAW_PAGE;
    }
    if (s_reader_panel == READER_PANEL_CHROME_SETTINGS || s_reader_panel == READER_PANEL_CHROME_PICKER) {
        bool picker = s_reader_panel == READER_PANEL_CHROME_PICKER;
        int top = picker ? 270 : KEY_SETTINGS_TOP;
        if (y < top) s_reader_panel = READER_PANEL_NONE;
        else if (ui_rect_hit((EpdRect){36, top + 29, 56, 56}, x, y) || ui_rect_hit((EpdRect){36, 1141, 612, 58}, x, y))
            s_reader_panel = picker ? READER_PANEL_CHROME_SETTINGS : READER_PANEL_REFRESH_SETTINGS;
        else if (picker) {
            for (int i = 0; i < READER_CHROME_ITEM_ROWS; ++i) {
                EpdRect card = chrome_pick_card(i);
                if (!chrome_row_visible(card, CHROME_PICK_TOP, CHROME_PICK_VIEW_HEIGHT) || !ui_rect_hit(card, x, y)) continue;
                if (!app_settings_set_reader_chrome_item(s_chrome_pick_bar, s_chrome_pick_slot, s_chrome_item_order[i])) {
                    copy_text(s_chrome_error, sizeof(s_chrome_error), "保存失败，请重试");
                    return APP_REDRAW_PAGE;
                }
                s_chrome_error[0] = 0;
                s_reader_panel = READER_PANEL_CHROME_SETTINGS;
                invalidate_prep();
                return APP_REDRAW_PAGE;
            }
            return APP_REDRAW_NONE;
        } else {
            for (int row = 0; row < CHROME_ROW_COUNT; ++row) {
                EpdRect card = chrome_row_rect(row);
                if (!chrome_row_visible(card, CHROME_ROW_TOP, CHROME_ROW_VIEW_HEIGHT) || !ui_rect_hit(card, x, y)) continue;
                if (row == CHROME_ROW_COUNT - 1) {
                    return apply_reader_chrome_heights(APP_READER_HEADER_HEIGHT_DEFAULT, APP_READER_FOOTER_HEIGHT_DEFAULT, true);
                }
                if (row >= 6) {
                    const unsigned bar = (unsigned)row - 6;
                    int next = s_chrome_height[bar];
                    if (x >= 338 && x < 410) next -= APP_READER_CHROME_HEIGHT_STEP;
                    else if (x >= 566 && x < 638) next += APP_READER_CHROME_HEIGHT_STEP;
                    else return APP_REDRAW_NONE;
                    const int min = bar ? APP_READER_FOOTER_HEIGHT_MIN : APP_READER_HEADER_HEIGHT_MIN;
                    const int max = bar ? APP_READER_FOOTER_HEIGHT_MAX : APP_READER_HEADER_HEIGHT_MAX;
                    if (next < min || next > max) return APP_REDRAW_NONE;
                    return apply_reader_chrome_heights(bar ? s_chrome_height[0] : next,
                                                       bar ? next : s_chrome_height[1], false);
                }
                s_chrome_pick_bar = (unsigned)row / 3;
                s_chrome_pick_slot = (unsigned)row % 3;
                s_chrome_error[0] = 0;
                s_chrome_pick_scroll = 0;
                s_chrome_pick_drag = false;
                s_reader_panel = READER_PANEL_CHROME_PICKER;
                invalidate_prep();
                return APP_REDRAW_PAGE;
            }
            return APP_REDRAW_NONE;
        }
        invalidate_prep();
        return APP_REDRAW_PAGE;
    }
    if (s_reader_panel == READER_PANEL_TURN_SETTINGS) {
        if (y < 270) s_reader_panel = READER_PANEL_NONE;
        else if (ui_rect_hit((EpdRect){36, 299, 56, 56}, x, y)) s_reader_panel = READER_PANEL_REFRESH_SETTINGS;
        else if (ui_rect_hit((EpdRect){36, 1141, 612, 58}, x, y)) s_reader_panel = READER_PANEL_REFRESH_SETTINGS;
        else {
            // 晃动翻页整行开关，矩形与 ui_reader_turn_settings_draw 的行保持一致。
            // The shake-turn toggle row; keep the rect in sync with ui_reader_turn_settings_draw.
            if (ui_rect_hit((EpdRect){36, 1000, 612, 88}, x, y)) {
                app_settings_set_reader_shake_turn(!app_settings_reader_shake_turn());
                invalidate_prep();
                return APP_REDRAW_PAGE;
            }
            for (int i = 0; i < 2; ++i) {
                if (!ui_rect_hit((EpdRect){36, 370 + i * 322, 612, 292}, x, y)) continue;
                app_settings_set_reader_vertical_turn(i != 0);
                invalidate_prep();
                return APP_REDRAW_PAGE;
            }
            return APP_REDRAW_NONE;
        }
        invalidate_prep();
        return APP_REDRAW_PAGE;
    }
    if (s_reader_panel == READER_PANEL_WEIGHT_SETTINGS) {
        if (y < 690 || y >= 1141) {
            s_reader_panel = y < 580 ? READER_PANEL_NONE : READER_PANEL_FONT_SETTINGS;
            return APP_REDRAW_PAGE;
        }
        int step=reader_weight_step(x,y),weight=app_settings_book_weight_fine();
        if(step) {
            int next=weight+step;if(next < -APP_BOOK_WEIGHT_FINE_MAX || next > APP_BOOK_WEIGHT_FINE_MAX)return APP_REDRAW_NONE;
            lock_draw();
            invalidate_prep();
            bool ok = app_settings_set_book_weight_fine(next);
            if (ok) ttf_set_stroke_adjust_fine(next);
            unlock_draw();
            if (!ok) {
                copy_text(s_reader_notice, sizeof(s_reader_notice), "保存失败，请重试");
                s_reader_notice_until = esp_timer_get_time() / 1000 + 1800;
            }
            return APP_REDRAW_PAGE;
        }
        return APP_REDRAW_NONE;
    }
    if (s_reader_panel == READER_PANEL_FONT_SETTINGS) {
        const int top = reader_font_top();
        EpdRect size = reader_slider_rect(0);
        EpdRect font = s_font_card;
        EpdRect layout = s_layout_card;
        EpdRect rule = s_rule_card;
        EpdRect bookfont = s_bookfont_card;
        if (ui_rect_hit((EpdRect){36, 1141, 612, 58}, x, y)) {
            s_reader_panel = READER_PANEL_NONE;
            invalidate_prep(); return APP_REDRAW_PAGE;
        }
        if (ui_rect_hit(size, x, y)) {
            int px = BOOK_PX_MIN + slider_index(size, x, BOOK_PX_MAX - BOOK_PX_MIN + 1);
            return apply_reader_layout(ctx, px, s_margin, app_settings_book_line_spacing(),
                                       app_settings_book_paragraph_spacing());
        }
        if (ui_rect_hit(font, x, y)) {
            ttf_font_scan();
            s_font_page = 0;
            int count = ttf_font_count();
            const char* active = ttf_font_path();
            for (int i = 0; i < count; ++i) {
                const ttf_font_item_t* item = ttf_font_item(i);
                if (item && !strcmp(active, item->path)) { s_font_page = i / BOOK_FONT_PAGE; break; }
            }
            s_reader_panel = READER_PANEL_FONT_PICKER;
            return APP_REDRAW_PAGE;
        }
        if (ui_rect_hit(s_weight_card, x, y)) {
            s_reader_panel = READER_PANEL_WEIGHT_SETTINGS;
            return APP_REDRAW_PAGE;
        }
        if (ui_rect_hit(s_antialias_card, x, y)) {
            lock_draw();
            invalidate_prep();
            bool on = !app_settings_book_antialias();
            bool ok = app_settings_set_book_antialias(on);
            if (ok) ttf_set_antialias(on);
            unlock_draw();
            if (!ok) {
                copy_text(s_reader_notice, sizeof(s_reader_notice), "保存失败，请重试");
                s_reader_notice_until = esp_timer_get_time() / 1000 + 1800;
            }
            return APP_REDRAW_PAGE;
        }
        if (ui_rect_hit(layout, x, y)) {
            s_reader_slider = -1;
            s_reader_panel = READER_PANEL_LAYOUT_SETTINGS;
            return APP_REDRAW_PAGE;
        }
        if (ui_rect_hit(rule, x, y)) {
            s_reader_panel = READER_PANEL_RULE_SETTINGS;
            return APP_REDRAW_PAGE;
        }
        if (ui_rect_hit(bookfont, x, y)) {
            // 触摸回调的栈比 on_tick 深得多：在这里直接重排一章会把主任务栈顶穿（实测溢出过一次）。
            // 只记下意愿，交给下一轮 on_tick 在浅栈上做。
            // The touch callback runs far deeper than on_tick, and laying a chapter out from here
            // overran the main task stack (measured once). Record the intent and let the next
            // on_tick do it on a shallow stack.
            bool on = !app_settings_book_fonts();
            app_settings_set_book_fonts(on);
            s_book_fonts_applied = on;
            book_set_embedded_fonts(on);
            s_fonts_reload_pending = true;
            invalidate_prep();
            return APP_REDRAW_PAGE;
        }
        s_reader_panel = y < top ? READER_PANEL_NONE : READER_PANEL_TOOLS;
        invalidate_prep();
        return APP_REDRAW_PAGE;
    }
    if (s_reader_panel == READER_PANEL_LAYOUT_SETTINGS) {
        const int top = READER_LAYOUT_SHEET_TOP;
        EpdRect back_icon = {36, top + 29, 56, 56};
        EpdRect back_button = reader_layout_back_rect();
        if (ui_rect_hit(back_icon, x, y) || ui_rect_hit(back_button, x, y)) {
            s_reader_slider = -1;
            s_reader_panel = READER_PANEL_FONT_SETTINGS;
            return APP_REDRAW_PAGE;
        }
        for (int i = 0; i < MARGIN_COUNT; ++i) {
            EpdRect margin = reader_slider_rect(1 + i);
            if (!ui_rect_hit(margin, x, y)) continue;
            int next[MARGIN_COUNT];
            memcpy(next, s_margin, sizeof(next));
            next[i] = reader_margin_input(margin, x, s_margin[i]);
            return apply_reader_layout(ctx, s_px, next, app_settings_book_line_spacing(),
                                       app_settings_book_paragraph_spacing());
        }
        EpdRect line = reader_slider_rect(5);
        EpdRect paragraph = reader_slider_rect(6);
        EpdRect tracking = reader_slider_rect(7);
        if (ui_rect_hit(line, x, y)) {
            uint8_t value = (uint8_t)(110 + slider_index(line, x, 41));
            return apply_reader_layout(ctx, s_px, s_margin, value,
                                       app_settings_book_paragraph_spacing());
        }
        if (ui_rect_hit(paragraph, x, y))
            return apply_reader_layout(ctx, s_px, s_margin, app_settings_book_line_spacing(),
                                       app_settings_book_paragraph_value((uint8_t)slider_index(paragraph, x, APP_BOOK_PARAGRAPH_COUNT)));
        if (ui_rect_hit(tracking, x, y))
            return apply_reader_typography(ctx, slider_index(tracking, x, APP_BOOK_TRACK_COUNT));
        for (int i = 0; i < 4; ++i) {
            if (ui_rect_hit(indent_choice_rect(i), x, y))
                return apply_reader_indent(ctx, i, app_settings_book_indent_adjust());
        }
        for (int i = 0; i < 3; ++i) {
            if (indent_adjust_hit(x,y)!=i) continue;
            int adjustment = i == 1 ? 0 : app_settings_book_indent_adjust() + (i == 0 ? -1 : 1);
            if (adjustment < -100) adjustment = -100;
            if (adjustment > 100) adjustment = 100;
            return apply_reader_indent(ctx, app_settings_book_indent(), adjustment);
        }
        if (ui_rect_hit(title_style_entry_rect(), x, y)) {
            s_reader_slider = -1;
            s_title_style_scroll = 0;
            s_title_style_drag = false;
            s_reader_panel = READER_PANEL_TITLE_STYLE;
            return APP_REDRAW_PAGE;
        }
        if (y < top) {
            s_reader_panel = READER_PANEL_NONE;
            invalidate_prep();
            return APP_REDRAW_PAGE;
        }
        return APP_REDRAW_NONE;
    }
    if (s_reader_panel == READER_PANEL_TITLE_STYLE) {
        EpdRect back_icon = {32, TITLE_STYLE_SHEET_TOP + 25, 64, 64};
        EpdRect back_button = {36, TITLE_STYLE_FOOTER_TOP, 612, 58};
        if (y < TITLE_STYLE_SHEET_TOP || ui_rect_hit(back_icon, x, y) || ui_rect_hit(back_button, x, y)) {
            s_reader_panel = READER_PANEL_LAYOUT_SETTINGS;
            return APP_REDRAW_PAGE;
        }
        // 槽位坐标已含滚动量；视口外的旧矩形不参与命中。
        // Slot coordinates already include the scroll offset; rects outside the viewport never hit.
        if (y < TITLE_STYLE_VIEW_TOP || y >= TITLE_STYLE_VIEW_BOTTOM) return APP_REDRAW_NONE;
        for (int i = 0; i < 3; ++i) {
            if (ui_rect_hit(heading_size_rect(i), x, y)) return apply_heading_size(i);
        }
        for (int i = 0; i < APP_BOOK_TITLE_STYLE_COUNT; ++i) {
            if (!ui_rect_hit(title_style_card_rect(i), x, y)) continue;
            // 样式只改首页题头的画法，不重排也不动预绘；收紧面板后重画当前页即可。
            // A style only changes how the first page's heading is painted, so nothing is
            // re-typeset and the prefetch stays; the next page repaint shows it.
            app_settings_set_book_title_style((uint8_t)i);
            return APP_REDRAW_PAGE;
        }
        return APP_REDRAW_NONE;
    }
    if (s_reader_panel == READER_PANEL_RULE_SETTINGS) {
        if (y < 729 || ui_rect_hit((EpdRect){36, 1113, 612, 72}, x, y)) {
            s_reader_panel = READER_PANEL_FONT_SETTINGS;
            return APP_REDRAW_PAGE;
        }
        for (int i = 0; i < 3; ++i) {
            if (!ui_rect_hit(rule_style_rect(i), x, y)) continue;
            app_settings_set_book_reading_line((uint8_t)i);
            book_layout_set_reading_line((unsigned)i);
            invalidate_prep();
            return APP_REDRAW_PAGE;
        }
        if (app_settings_book_reading_line()) {
            int offset = app_settings_book_reading_line_offset();
            if (ui_rect_hit(s_rule_offset_up, x, y)) offset -= 2;
            else if (ui_rect_hit(s_rule_offset_reset, x, y)) offset = 0;
            else if (ui_rect_hit(s_rule_offset_down, x, y)) offset += 2;
            else return APP_REDRAW_NONE;
            if (offset < -8) offset = -8;
            if (offset > 8) offset = 8;
            app_settings_set_book_reading_line_offset((int8_t)offset);
            book_layout_set_reading_line_offset(offset);
            invalidate_prep();
            return APP_REDRAW_PAGE;
        }
        return APP_REDRAW_NONE;
    }
    if (s_reader_panel == READER_PANEL_FONT_PICKER) {
        const int top = reader_font_picker_top();
        if (y < top + 112) {
            s_reader_panel = READER_PANEL_FONT_SETTINGS;
            return APP_REDRAW_PAGE;
        }
        int count = ttf_font_count(), start = s_font_page * BOOK_FONT_PAGE;
        for (int slot = 0; slot < BOOK_FONT_PAGE && start + slot < count; ++slot) {
            EpdRect card = reader_font_pick_rect(slot);
            if (ui_rect_hit(card, x, y)) return select_reading_font_item(ctx, ttf_font_item(start + slot));
        }
        return APP_REDRAW_NONE;
    }
    return APP_REDRAW_NONE;
}
static app_redraw_t manage_action(app_ctx_t* ctx, uint16_t x, uint16_t y) {
    if (ui_rect_hit(manage_back_rect(), x, y)) {
        if (s_clear_confirm) s_clear_confirm = false;
        else s_view = SHELF;
        return APP_REDRAW_PAGE;
    }
    if (!s_clear_confirm && !s_file_removed && ui_rect_hit((EpdRect){UI_MARGIN, manage_panel().y + manage_panel().height - 258, ui_content_width(), 58}, x, y)) {
        category_begin(ctx,true,false); return APP_REDRAW_PAGE;
    }
    if (!s_clear_confirm && !s_file_removed && ui_rect_hit((EpdRect){UI_MARGIN, manage_panel().y + manage_panel().height - 190, ui_content_width(), 68}, x, y)) {
        extern void app_files_request_folder(int folder);
        app_files_request_folder(0);
        ui_nav_request(ctx, 2);
        return APP_REDRAW_NONE;
    }
    int count = s_clear_confirm || s_file_removed ? 2 : 4;
    for (int i = 0; i < count; ++i) if (ui_rect_hit(manage_rect(i, count), x, y)) {
        if (s_clear_confirm) { if (!i) s_clear_confirm = false; else manage_apply(ctx); }
        else if (!i) s_view = SHELF;
        else if (s_file_removed) manage_apply(ctx);
        else if (i == 1) {
            // 收藏开关与封面角标、收藏置顶共用一份 NVS 数据，保存后即时重排。
            // The favorite toggle shares the NVS data behind cover badges and favorite-first
            // sorting, resorting the shelf as soon as the save succeeds.
            if (!favorite_save(s_managed.path, !s_managed.favorite)) {
                copy_text(s_manage_message, sizeof(s_manage_message), "收藏保存失败");
            } else {
                s_managed.favorite = !s_managed.favorite;
                for (int n = 0; n < s_count; ++n)
                    if (!strcmp(s_shelf[n].path, s_managed.path)) s_shelf[n].favorite = s_managed.favorite;
                sort_shelf(ctx);
            }
        }
        else { s_clear_confirm = true; s_delete_confirm = i == 3; s_manage_message[0] = 0; }
        break;
    }
    return APP_REDRAW_PAGE;
}
static app_redraw_t editor_save(app_ctx_t* ctx) {
    if (ui_keyboard_pending()) {
        copy_text(s_editor_notice, sizeof(s_editor_notice), "请先选择候选字");
        return APP_REDRAW_PAGE;
    }
    char *start = s_editor_title;
    while (*start == ' ') ++start;
    if (start != s_editor_title) memmove(s_editor_title, start, strlen(start) + 1);
    size_t len = strlen(s_editor_title);
    while (len && s_editor_title[len - 1] == ' ') s_editor_title[--len] = 0;
    if (!s_editor_title[0]) {
        copy_text(s_editor_notice, sizeof(s_editor_notice), "书名不能为空");
        return APP_REDRAW_PAGE;
    }
    esp_err_t err = book_title_set(s_managed.path, s_editor_title);
    if (err != ESP_OK) {
        copy_text(s_editor_notice, sizeof(s_editor_notice), "保存失败，请检查书名或存储");
        return APP_REDRAW_PAGE;
    }
    for (int i = 0; i < s_count; ++i)
        if (!strcmp(s_shelf[i].path, s_managed.path)) copy_text(s_shelf[i].name, sizeof(s_shelf[i].name), s_editor_title);
    free(s_editor_cover); s_editor_cover = NULL;
    ui_keyboard_end();
    s_view = SHELF;
    sort_shelf(ctx);
    return APP_REDRAW_PAGE;
}
static app_redraw_t editor_paint(app_ctx_t* ctx, bool field) {
    bool search = s_view == SEARCH;
    ui_keyboard_update_t update = ui_keyboard_update(ctx->fb, 560,
        search ? (EpdRect){UI_MARGIN, 200, ui_content_width(), 88} : (EpdRect){36, 424, 612, 83},
        search ? UI_PX_BODY : 31, false, NULL, field);
    s_area = update.area; s_mode = update.layout ? MODE_GL16 : MODE_DU; s_input_settle = update.settle;
    return update.area.width ? APP_REDRAW_AREA : APP_REDRAW_NONE;
}
static app_redraw_t editor_action(app_ctx_t* ctx, uint16_t x, uint16_t y) {
    if (y < 134) {
        if (x < 160) { ui_keyboard_end(); free(s_editor_cover); s_editor_cover = NULL; s_view = SHELF; return APP_REDRAW_PAGE; }
        if (x > 510) return editor_save(ctx);
        return APP_REDRAW_NONE;
    }
    if (ui_text_input_tap(&s_editor_input, (EpdRect){36, 424, 612, 83}, 31,
                          false, NULL, x, y)) return editor_paint(ctx, true);
    ui_keyboard_result_t result = ui_keyboard_tap(x, y, 560, ctx->now_ms);
    if (result == UI_KEYBOARD_DONE) return editor_save(ctx);
    return result == UI_KEYBOARD_CHANGED ? editor_paint(ctx, false) : APP_REDRAW_NONE;
}
// 成功项退出选择，已删文件的失败项仅重试元数据。/ Deselect successes; removed-file failures retry metadata only.
static void batch_apply(app_ctx_t* ctx) {
    const bool unshelf = s_batch_kind == BATCH_UNSHELF;
    book_shelf_management_t management = shelf_management_context();
    book_shelf_manage_kind_t kind = unshelf ? BOOK_SHELF_HIDE :
        s_batch_kind == BATCH_DELETE ? BOOK_SHELF_DELETE : BOOK_SHELF_CLEAR;
    book_shelf_batch_result_t result = book_shelf_manage_batch(&management, s_shelf, &s_count,
                                                             kind, shelf_hidden_save);
    unsigned done = result.done, failed = result.failed;
    refresh_capacity();
    int leaf = ctx->leaf;
    sort_shelf(ctx);
    ctx->leaf = leaf < leaves() ? leaf : leaves() - 1;
    s_batch_confirm = false;
    if (unshelf)
        snprintf(s_batch_message, sizeof(s_batch_message), "已移出书架 %u 本，失败 %u 本%s",
                 done, failed, failed ? "；所选可重试" : "");
    else
        snprintf(s_batch_message, sizeof(s_batch_message), "成功 %u 本，失败 %u 本%s",
                 done, failed, failed ? "；所选可重试" : "");
}
static app_redraw_t batch_action(app_ctx_t* ctx, uint16_t x, uint16_t y) {
    if (s_batch_confirm) {
        if (ui_rect_hit(ui_row_rect(0, 2, 620, UI_BTN_H), x, y)) {
            s_batch_confirm = false;
            return APP_REDRAW_PAGE;
        }
        if (ui_rect_hit(ui_row_rect(1, 2, 620, UI_BTN_H), x, y)) {
            batch_apply(ctx);
            return APP_REDRAW_PAGE;
        }
        return APP_REDRAW_NONE;
    }
    for (int i = 0; i < 6; ++i) if (ui_rect_hit(batch_rect(i), x, y)) {
        s_batch_message[0] = 0;
        if (!i) select_page(ctx->leaf);
        else if (i == 1) clear_selection();
        else if (i == 2) { clear_selection(); s_batch_message[0] = 0; scan_shelf(ctx); }
        else if (selected_count()) {
            s_batch_kind = i == 3 ? BATCH_DELETE : i == 4 ? BATCH_CLEAR : BATCH_UNSHELF;
            s_batch_confirm = true;
        }
        else copy_text(s_batch_message, sizeof(s_batch_message), "请先选择图书");
        return APP_REDRAW_PAGE;
    }
    return APP_REDRAW_NONE;
}
static app_redraw_t bulk_turn_page(app_ctx_t* ctx, int direction) {
    int next = ctx->leaf + direction;
    if (s_scan_pending || s_batch_confirm || next < 0 || next >= leaves()) return APP_REDRAW_NONE;
    ctx->leaf = next;
    return APP_REDRAW_PAGE;
}
static app_redraw_t bulk_finish(app_ctx_t* ctx) {
    int first = ctx->leaf * BOOK_BULK_ROWS;
    clear_selection();
    s_batch_message[0] = 0;
    s_batch_confirm = false;
    s_view = SHELF;
    ctx->leaf = first / shelf_rows();
    if (ctx->leaf >= leaves()) ctx->leaf = leaves() - 1;
    return APP_REDRAW_PAGE;
}
// 批量管理单独路由，不复用带示例菜单的旧底栏。
// Route batch management independently of the legacy bar and demo menu.
static app_redraw_t bulk_action(app_ctx_t* ctx, uint16_t x, uint16_t y) {
    if (!s_batch_confirm && ui_rect_hit((EpdRect){584,79,64,60},x,y)) {
        if (!selected_count()) copy_text(s_batch_message,sizeof(s_batch_message),"请先选择图书");
        else category_begin(ctx,true,true);
        return APP_REDRAW_PAGE;
    }
    if (ui_rect_hit(manage_back_rect(), x, y)) {
        if (s_batch_confirm) { s_batch_confirm = false; return APP_REDRAW_PAGE; }
        return bulk_finish(ctx);
    }
    app_redraw_t result = batch_action(ctx, x, y);
    if (result != APP_REDRAW_NONE || s_batch_confirm || s_scan_pending) return result;
    for (int i = 0; i < 3; ++i) if (ui_rect_hit(bulk_filter_rect(i), x, y)) {
        if (i == 2) { search_begin(); return APP_REDRAW_PAGE; }
        if (i == 0) s_filter = (s_filter + 1) % 3;
        else { s_recent_sort = !shelf_query().recent_sort; book_shelf_categories_set_sort(&s_shelf_categories,s_recent_sort); app_settings_set_shelf_recent_sort(s_recent_sort); }
        clear_selection(); s_batch_message[0] = 0;
        sort_shelf(ctx);
        return APP_REDRAW_PAGE;
    }
    int nav = bulk_nav_hit(x, y);
    if (nav == 1) return bulk_finish(ctx);
    if (nav == 0 || nav == 2) return bulk_turn_page(ctx, nav == 0 ? -1 : 1);
    for (int row = 0; row < BOOK_BULK_ROWS; ++row) {
        int index = ctx->leaf * BOOK_BULK_ROWS + row;
        if (index < s_visible_count && ui_rect_hit(row_rect(row), x, y)) {
            toggle_selection(index); s_batch_message[0] = 0;
            return APP_REDRAW_PAGE;
        }
    }
    return APP_REDRAW_NONE;
}
static int bulk_control_at(app_ctx_t* ctx, uint16_t x, uint16_t y, EpdRect* rect) {
    *rect = manage_back_rect();
    if (ui_rect_hit(*rect, x, y)) return 711;
    if (s_batch_confirm) {
        for (int i = 0; i < 2; ++i) {
            *rect = ui_row_rect(i, 2, 620, UI_BTN_H);
            if (ui_rect_hit(*rect, x, y)) return 600 + i;
        }
        return -1;
    }
    *rect=(EpdRect){584,79,64,60}; if(ui_rect_hit(*rect,x,y))return 718;
    for (int i = 0; i < 3; ++i) {
        *rect = bulk_filter_rect(i);
        if (ui_rect_hit(*rect, x, y)) return 110 + i;
        *rect = bulk_nav_rect(i);
        if (ui_rect_hit(*rect, x, y)) return 100 + i;
    }
    for (int i = 0; i < 6; ++i) {
        *rect = batch_rect(i);
        if (ui_rect_hit(*rect, x, y)) return 610 + i;
        if (ctx->leaf * BOOK_BULK_ROWS + i < s_visible_count) {
            *rect = row_rect(i);
            if (ui_rect_hit(*rect, x, y)) return i;
        }
    }
    return -1;
}
static app_redraw_t action_at(app_ctx_t* ctx, uint16_t x, uint16_t y) {
    if (s_view == BULK) return bulk_action(ctx, x, y);
    if (s_view == IMPORT) {
        if (y < 160 && x < 120) { s_view = SHELF; return APP_REDRAW_PAGE; }
        int tab = ui_nav_hit(x, y);
        if (tab == 1) { s_view = SHELF; return APP_REDRAW_PAGE; }
        if (tab >= 0) { ui_nav_request(ctx, tab); return APP_REDRAW_NONE; }
        for (int i = 0; i < 4; ++i) if (ui_rect_hit(import_rect(i), x, y)) {
            if (i == 0) {
                extern const app_desc_t app_files;
                ctx->request_app = &app_files;
            } else {
                extern const app_desc_t app_transfer;
                if (i == 1) app_transfer_request_wifi_upload();
                else if (i == 2) app_transfer_request_hotspot_start();
                else app_transfer_request_usb_start();
                ctx->request_app = &app_transfer;
            }
            return APP_REDRAW_NONE;
        }
        return APP_REDRAW_NONE;
    }
    if (s_view == TOC) {
        if (s_toc_jump_open) {
            int choice = book_toc_jump_hit(x, y);
            if (choice == BOOK_TOC_JUMP_TRACK) {
                s_toc_jump_percent = book_toc_jump_percent(x);
                return APP_REDRAW_PAGE;
            }
            bool confirmed = choice == BOOK_TOC_JUMP_CONFIRM;
            s_toc_jump_open = s_toc_jump_drag = false;
            if (confirmed) {
                // 百分比是全书进度：确认后把正文移到该位置并回到阅读页，而不是只翻目录页。
                // The percentage is whole-book progress: confirming moves the body there and returns to
                // the reader, instead of only turning TOC pages.
                size_t chapter = 0, source = 0;
                if (reader_percent_target(s_toc_jump_percent, &chapter, &source)) {
                    ESP_LOGI(TAG, "jump target pct=%d chapter=%u source=%zu",
                             s_toc_jump_percent, (unsigned)chapter, source);
                    save_progress();
                    set_reader_view(READING);
                    s_reader_panel = READER_PANEL_NONE;
                    if (load_chapter_at(ctx, chapter, 0, false, NULL, SIZE_MAX)) {
                        // 章内按页扫描：percent() 随页单调递增，取不超过目标的最后一页。
                        // 这样不依赖各格式的 anchor/source 偏移映射，TXT 与非 MOBI 也走同一条路。
                        // Scan pages: percent() rises with the page, so the last page at or below the
                        // target is the landing spot. No per-format anchor mapping is involved, which
                        // keeps TXT and other backends on the same path.
                        size_t pages = book_layout_page_count(), best = 0;
                        // 0% 单独钉在第 1 页：全书前 1% 的字节可能铺满好几页，它们的 percent
                        // 都是 0，按"最后一个 ≤ 目标"会把 0% 落到前言中部而不是封面。
                        // Pin 0% to the first page: the first 1% of the book can span several
                        // pages that all read percent 0, and "last page at or below" would drop
                        // the jump mid-front-matter instead of on the cover.
                        if (s_toc_jump_percent > 0)
                            for (size_t p = 0; p < pages; ++p) {
                                if ((int)percent(p) > s_toc_jump_percent) break;
                                best = p;
                            }
                        s_reader_session.position.page = best;
                        s_selected_toc = SIZE_MAX;
                        ESP_LOGI(TAG, "jump landed page=%u/%u pct=%u",
                                 (unsigned)best + 1, (unsigned)pages, percent(best));
                        save_progress();
                    }
                }
            }
            return APP_REDRAW_PAGE;
        }
        int target = book_toc_hit(x, y, ctx->leaf, book_navigation_count());
        if (target == BOOK_TOC_BACK) {
            s_message[0] = 0;
            set_reader_view(READING);
            s_reader_panel = READER_PANEL_NONE;
            return APP_REDRAW_PAGE;
        }
        if (target == BOOK_TOC_PREV || target == BOOK_TOC_NEXT ||
            target == BOOK_TOC_SKIP_PREV || target == BOOK_TOC_SKIP_NEXT) {
            int step = target == BOOK_TOC_SKIP_PREV || target == BOOK_TOC_SKIP_NEXT ? 10 : 1;
            int next = ctx->leaf + (target == BOOK_TOC_PREV || target == BOOK_TOC_SKIP_PREV ? -step : step);
            if (next < 0) next = 0;
            if (next >= leaves()) next = leaves() - 1;
            if (next != ctx->leaf) { ctx->leaf = next; return APP_REDRAW_PAGE; }
            return APP_REDRAW_NONE;
        }
        if (target == BOOK_TOC_JUMP && book_navigation_count()) {
            s_toc_jump_open = true;
            // 打开时对齐当前阅读位置，和正文底部进度条显示同一个数。
            // Seed from the current reading position so it matches the reader's footer bar exactly.
            s_toc_jump_percent = reader_book_percent();
            return APP_REDRAW_PAGE;
        }
        if (target >= 0 && !s_message[0]) {
            size_t chapter = book_navigation_chapter((size_t)target);
            if (chapter >= book_chapter_count()) return APP_REDRAW_NONE;
            // 目录条目落点链路单独留痕：章、章内偏移与标题一次点按即可核对。
            // One log per entry tap pins the landing chain: chapter, in-segment offset and title.
            char nav_title[128] = {0};
            (void)book_navigation_title((size_t)target, nav_title, sizeof(nav_title));
            ESP_LOGI(TAG, "toc entry=%u chapter=%u source=%zu title=%s", (unsigned)target,
                     (unsigned)chapter, book_navigation_source_offset((size_t)target),
                     nav_title[0] ? nav_title : "-");
            save_progress();
            set_reader_view(READING);
            if (load_chapter_at(ctx, chapter, 0, false, book_navigation_anchor((size_t)target),
                                book_navigation_source_offset((size_t)target))) {
                s_selected_toc = (size_t)target;
                char entry_title[sizeof(s_reader_session.book.chapter_title)];
                if (book_navigation_title((size_t)target, entry_title, sizeof(entry_title)) == ESP_OK)
                    copy_text(s_reader_session.book.chapter_title, sizeof(s_reader_session.book.chapter_title), entry_title);
                s_view = READING;
                s_reader_panel = READER_PANEL_NONE;
                save_progress();
            } else set_reader_view(s_text ? TOC : SHELF);
            return APP_REDRAW_PAGE;
        }
        return APP_REDRAW_NONE;
    }
    if (s_view == MANAGE) return manage_action(ctx, x, y);
    if (s_view == SEARCH) {
        if (y < 160 && x < 160) { search_finish(ctx, false); return APP_REDRAW_PAGE; }
        if (ui_text_input_tap(&s_search_input, (EpdRect){UI_MARGIN, 200, ui_content_width(), 88},
                              UI_PX_BODY, false, NULL, x, y)) return editor_paint(ctx, true);
        if (ui_rect_hit((EpdRect){36, 392, 180, 66}, x, y)) { s_search_draft[0] = 0; ui_text_edit_init(&s_search_input, s_search_draft, sizeof(s_search_draft)); ui_keyboard_begin(&s_search_input, false); return APP_REDRAW_PAGE; }
        ui_keyboard_result_t result = ui_keyboard_tap(x, y, 560, ctx->now_ms);
        if (result == UI_KEYBOARD_DONE || (y < 160 && x > 510 && !ui_keyboard_pending())) { search_finish(ctx, true); return APP_REDRAW_PAGE; }
        if (result == UI_KEYBOARD_CHANGED) return editor_paint(ctx, false);
        return APP_REDRAW_NONE;
    }
    if (s_view == READING) {
        // 工具栏展开时点按正文只收起工具栏，不再顺带翻页；其他面板仍按各自规则处理。
        // A body tap with the tools bar open only dismisses it instead of also turning; the
        // other panels keep their own rules.
        if (s_toolbar) return reader_panel_action(ctx, x, y);
        // 脚注标记优先于翻页热区：它常落在正文右三分之一，先判内链再判翻页。
        // Footnote markers win over the turn zones: they often sit in the right third, so the
        // link test runs before the page-turn test.
        int link = book_layout_link_at(s_reader_session.position.page, x, y);
        if (link >= 0 && note_open_link(link)) return APP_REDRAW_PAGE;
        // 页眉页脚槽位只显示内容，无控件；点信息条不翻页，落入下方正文热区判定。
        // The chrome slots show content only, no controls; bar taps do not turn pages and fall
        // through to the body hit test below.
        // 通栏插图铺满整屏，点在图两侧的边条上同样要翻页；文字页仍按正文栏判定。
        // A full-bleed illustration spans the panel, so taps on its side strips must turn pages
        // too; text pages keep the body column.
        EpdRect tap_area = body_rect();
        if (reader_image_slots(s_reader_session.position.page)) tap_area = ui_rect_union(tap_area, book_layout_image_rect());
        if (!ui_rect_hit(tap_area, x, y)) return APP_REDRAW_NONE;
        int target = book_reader_tap_direction(tap_area, x, y,
                                              app_settings_reader_vertical_turn(),
                                              app_settings_book_vertical());
#if CONFIG_TOUCH_DIAG
        TD_LOGI("TAPZ out=%d", target);
#endif
        if (target) return turn_page(ctx, target);
        return APP_REDRAW_NONE;
    }
    if (s_view == SHELF) {
        if(s_shelf_selecting) {
            int source=book_shelf_categories_selected(&s_shelf_categories);
            if(ui_rect_hit(book_selection_rect(BOOK_SELECTION_REVIEW),x,y)){
                category_start_books(ctx,BOOK_COLLECTION_ALL,true);return APP_REDRAW_PAGE;
            }
            if(ui_rect_hit(book_selection_rect(BOOK_SELECTION_SUBMIT),x,y)){
                if(!selected_count())return APP_REDRAW_NONE;
                category_begin(ctx,true,true);return APP_REDRAW_PAGE;
            }
            if(source>=2 && selected_count() && ui_rect_hit(book_selection_rect(BOOK_SELECTION_MORE),x,y)){
                category_begin(ctx,true,true);s_category_ui.source=book_shelf_categories_id(&s_shelf_categories,source);
                s_category_ui.screen=CATEGORY_SELECTED_ACTIONS;
                return APP_REDRAW_PAGE;
            }
            if(ui_rect_hit(book_selection_rect(BOOK_SELECTION_PAGE_ALL),x,y)){
                if(!s_visible_count)return APP_REDRAW_NONE;
                book_shelf_select_page(s_shelf,s_visible_count,ctx->leaf,shelf_rows());return APP_REDRAW_PAGE;
            }
        }
        if(!s_shelf_selecting && s_category_result[0] && book_collection_can_undo(&s_collection) &&
           ui_rect_hit((EpdRect){568,1056,80,56},x,y)){
            category_begin(ctx,false,false);s_category_quick=true;s_category_ui.command=CATEGORY_UNDO;return APP_REDRAW_PAGE;
        }
        if (s_view == SHELF) {
            int tab = s_shelf_selecting?-1:ui_nav_hit(x,y);
            if (tab >= 0) { ui_nav_request(ctx, tab); return APP_REDRAW_NONE; }
            int pager=s_shelf_selecting?book_selection_pager_hit(x,y):ui_pager_hit(x,y);
            if(pager>=0)return shelf_turn_page(ctx,ui_pager_target(pager,ctx->leaf,leaves())-ctx->leaf);
        }
        if (s_view == SHELF && ui_rect_hit(shelf_import_rect(), x, y)) {
            s_shelf_selecting=!s_shelf_selecting;clear_selection();
            return APP_REDRAW_PAGE;
        }
        if (s_view == SHELF && ui_rect_hit(shelf_manage_rect(), x, y)) {
            search_begin();
            return APP_REDRAW_PAGE;
        }
        if(!s_shelf_selecting && ui_rect_hit(shelf_header_action_rect(2),x,y)){
            clear_selection();s_batch_confirm=false;s_batch_message[0]=0;s_view=BULK;ctx->leaf=0;
            return APP_REDRAW_PAGE;
        }
        if(!s_shelf_selecting && ui_rect_hit(shelf_header_action_rect(3),x,y)){
            s_view=IMPORT;return APP_REDRAW_PAGE;
        }
        if(y>=160 && y<192) {
            int category=book_shelf_categories_selected(&s_shelf_categories);
            if(!s_shelf_selecting && x<336){
                bool any=false;for(int n=0;n<s_count;++n)if(s_shelf[n].new_book&&!s_shelf[n].removed)any=true;
                if(any){clear_selection();for(int n=0;n<s_count;++n)s_shelf[n].selected=s_shelf[n].new_book&&!s_shelf[n].removed;category_begin(ctx,true,true);return APP_REDRAW_PAGE;}
            }
            if(!s_shelf_selecting && category>=2 && x>=400){category_start_books(ctx,book_shelf_categories_id(&s_shelf_categories,category),false);return APP_REDRAW_PAGE;}
        }
        if (s_view == SHELF && s_save_failed && y >= UI_CONTENT_BOTTOM - UI_PX_CAPTION && y < UI_CONTENT_BOTTOM) {
            retry_progress();
            return APP_REDRAW_PAGE;
        }
    }
    if (s_message[0] && !((s_view == SHELF || s_view == BULK) && s_visible_count)) return APP_REDRAW_NONE;
    int rows = s_view == BULK ? BOOK_BULK_ROWS : shelf_rows();
    for (int row = 0; row < rows; ++row) if (ui_rect_hit(row_rect(row), x, y)) {
        int i = ctx->leaf * rows + row;
        if (s_view == SHELF && i < s_visible_count) {
            if(s_shelf_selecting){toggle_selection(i);return APP_REDRAW_PAGE;}
            shelf_category_remember(ctx);
            if (s_shelf[i].removed) {
                s_managed = s_shelf[i]; s_view = MANAGE; s_file_removed = s_delete_confirm = true; s_clear_confirm = false;
                copy_text(s_manage_message, sizeof(s_manage_message), "文件已删除，进度清理失败，请重试");
            } else open_book(ctx, s_shelf[i].path);
        }
        return APP_REDRAW_PAGE;
    }
    return APP_REDRAW_NONE;
}
static void on_enter(app_ctx_t* ctx) {
    category_reset();
    s_shelf_selecting=false;s_category_result[0]=0;
    reader_indent_hold_cancel();
    s_shelf_key_mode=s_shelf_key_paint_pending=false;s_shelf_key_delta=s_shelf_key_pressed=0;
    shelf_category_init();
    book_shelf_categories_menu_close(&s_shelf_categories);
    s_shelf_page_pending = false;
    // 设置页试按的旧事件不得在开书后翻页。/ Discard remote tests from earlier pages before opening a book.
    ble_pt_event_t old_key;
    ble_pt_raw_t old_raw;
    while (ble_pt_pop_key(&old_key)) {}
    while (ble_pt_pop_raw(&old_raw)) {}
    book_layout_set_spacing(app_settings_book_line_spacing(), app_settings_book_paragraph_spacing());
    book_layout_set_images_visible(!app_settings_reader_hide_images());
    // 插图通栏：无背景时整屏宽是面板常量，进入阅读页说一次，之后每次重排都沿用同一条带。
    // 有阅读背景时不开通栏：背景内框靠用户调边距对齐，插图必须跟着正文栏，否则会压到框上。
    // 背景只能在设置页开关，离开阅读页会 free_book，回页重开时本句按新状态重新发布。
    // Full-bleed illustrations: without a background the panel width is a constant, so publish
    // it once per entry and every later re-layout reuses the same band. With a reading
    // background there is no bleed: its inner frame is aligned by the user's margins, so
    // illustrations must stay inside the body column instead of crossing that frame. The
    // background only changes on the settings page, and leaving this page frees the book, so
    // re-entry republishes from the current state.
    book_layout_set_image_bleed_width(app_settings_book_background_path()[0] ? 0 : UI_LOCK_WIDTH);
    // 竖排是阅读设置里的手动开关，进页时同步给排版；关着就是原来的横排。
    // Vertical setting is a manual reader switch, published to the layout on entry; off is the
    // original horizontal setting.
    book_layout_set_vertical(app_settings_book_vertical());
    book_layout_set_typography(((int)app_settings_book_tracking() + APP_BOOK_TRACK_MIN));
    book_layout_set_first_line_indent(app_settings_book_indent());
    book_layout_set_first_line_indent_adjust(app_settings_book_indent_adjust());
    book_layout_set_reading_line(app_settings_book_reading_line());
    book_layout_set_reading_line_offset(app_settings_book_reading_line_offset());
    // 字体页可能刚改过书内字体开关；回正文时重排当前章，别让用户对着旧排版猜。
    // The font page may have just changed the embedded-face switch; re-typeset this chapter
    // so the reader is not left guessing at stale type.
    bool book_fonts = app_settings_book_fonts();
    if (book_fonts != s_book_fonts_applied) {
        s_book_fonts_applied = book_fonts;
        book_set_embedded_fonts(book_fonts);
        if (s_text && book_kind() == BOOK_KIND_EPUB)
            (void)load_chapter(ctx, s_reader_session.position.chapter, book_layout_page_start_offset(s_reader_session.position.page), false);
    }
    s_reader_fullscreen = false;
    s_bookmark_edit = s_bookmark_delete_confirm = s_bookmark_delete_error = false;
    s_bookmark_selected = 0;
    clear_selection();
    s_view = s_requested_manage ? BULK : SHELF;
    s_reader_return_home = false;
    s_requested_manage = false;
    s_presented_view = -1;
    ctx->leaf = s_view == SHELF ? book_shelf_categories_saved_page(&s_shelf_categories, app_settings_shelf_style() == 5) : 0;
    s_px = app_settings_book_px();
    s_chrome_height[0] = app_settings_reader_chrome_height(0);
    s_chrome_height[1] = app_settings_reader_chrome_height(1);
    s_recent_sort = app_settings_shelf_recent_sort();
    s_margin[MARGIN_LEFT] = app_settings_book_margin_left();
    s_margin[MARGIN_RIGHT] = app_settings_book_margin_right();
    s_margin[MARGIN_TOP] = app_settings_book_margin_top();
    s_margin[MARGIN_BOTTOM] = app_settings_book_margin_bottom();
    // 悬挂标点的右侧余量跟正文右边距走，读到四值后再发布，之后每次重排同步。
    // Publish the hanging-punctuation slack from the body's right inset once the four values are
    // loaded; every later re-layout keeps it in sync.
    book_layout_set_right_slack(s_margin[MARGIN_RIGHT]);
    s_line_spacing = app_settings_book_line_spacing();
    // 传书页已停止并 join HTTP，安全注销且只丢弃对应路径的旧进度。
    // Transfer has stopped and joined HTTP; safely unregister only invalidated paths.
    pending_drop_invalidated();
    s_reader_panel = READER_PANEL_NONE;
    s_reader_slider = -1;
    s_clear_confirm = s_batch_confirm = false;
    s_pressed_control = -1;
    s_du_count = 0;
    s_shelf_feedback_pending = false;
    s_cover_batch_present_pending = false;
    s_reader_cleanup = false;
    // 进页本身不整屏刷；只有真的开了书，首帧才需要擦掉上一页。
    // Entering the app alone does not force a full refresh; only an actually opened book does.
    s_reader_entry_full = false;
    s_poll_ms = 0;
    board_sd_info_t sd = {0};
    esp_err_t media_err = board_sd_get_info(&sd);
    bool reuse = s_shelf_cache_valid && media_err != ESP_ERR_NOT_FINISHED &&
                 !strcmp(s_cache_library,sd.mounted?sd.identity:"builtin") &&
                 sd.present == s_cache_sd_present && sd.mounted == s_cache_sd_mounted &&
                 s_cache_shelf_style == app_settings_shelf_style() &&
                 s_store_revision == book_store_revision();
    s_scan_pending = !reuse;
    if (reuse) {
        refresh_cached_progress(ctx);
        (void)open_requested_book(ctx);
    } else {
        s_count = s_visible_count = 0;
        s_message[0] = 0;
        s_storage[0] = 0;
        invalidate_covers();
        // 已指定文件可直接打开；书架目录留到真正进入书架时再扫描。
        // Open a requested file directly; scan the shelf only when it is actually shown.
        if (open_requested_book(ctx)) {
            s_scan_pending = false;
            return;
        }
        board_sd_start_probe();
    }

}
static void book_on_exit(app_ctx_t* ctx) {
    category_reset();
    s_shelf_selecting=false;clear_selection();s_category_result[0]=0;book_collection_drop_undo(&s_collection);
    track_ticket_stats(ctx);
    middle_pending_clear();
    reader_indent_hold_cancel();
    // 离页恢复加速度计静默；门槛与静默窗口一并复位，下次进书重新起步。
    // Silence the accelerometer on exit; reset the gate and touch window so the next session starts clean.
    if (s_shake_sensor_on && ctx->acc) board_sensor_sleep(ctx->acc);
    s_shake_sensor_on = ctx->acc && sc7a20h_powered(ctx->acc);
    s_shake_cfg_pending = true;
    s_shake_gate = (book_shake_gate_t){0};
    s_shake_touch_ms = 0;
    s_shelf_key_mode=s_shelf_key_paint_pending=false;s_shelf_key_delta=s_shelf_key_pressed=0;
    book_shelf_covers_cancel(&s_shelf_covers);
    shelf_category_remember(ctx);
    book_shelf_categories_menu_close(&s_shelf_categories);
    s_shelf_page_pending = false;
    ui_keyboard_end();
    s_presented_view = -1;
    free(s_editor_cover); s_editor_cover = NULL;
    s_pressed_control = -1;
    s_reader_slider = -1;
    save_progress();
    s_reader_return_home = false;
    ui_reader_background_release();
    free_book();
    reader_prefetch_release();
#if CONFIG_READER_PERF && CONFIG_READER_MEMORY
    reader_memory_snapshot("reader_exit");
#endif
}
// 先停止使用旧卡句柄与字体预渲染，主循环随后切换内置字体。
// Stop old-card handles and font preparation before the loop switches to the builtin font.
static void book_on_media_lost(app_ctx_t* ctx) {
    category_reset();
    s_shelf_selecting=false;s_category_result[0]=0;book_collection_drop_undo(&s_collection);
    middle_pending_clear();
    reader_indent_hold_cancel();
    s_shelf_key_mode=s_shelf_key_paint_pending=false;s_shelf_key_delta=s_shelf_key_pressed=0;
    shelf_category_remember(ctx);
    book_shelf_categories_menu_close(&s_shelf_categories);
    lock_draw();
    save_progress();
    free_book();
    unlock_draw();
    free(s_editor_cover); s_editor_cover = NULL;
    s_view = SHELF;
    s_count = s_visible_count = 0;
    s_shelf_cache_valid = false;
    free(s_shelf); s_shelf = NULL; s_shelf_capacity = 0;
    invalidate_covers();
    ctx->leaf = 0;
    s_reader_panel = READER_PANEL_NONE;
    s_reader_slider = -1;
    s_clear_confirm = s_batch_confirm = false;
    s_pressed_control = -1;
    s_scan_pending = true;
    s_du_count = 0;
    copy_text(s_storage, sizeof(s_storage), "TF 卡已移除");
    copy_text(s_message, sizeof(s_message), "TF 卡已移除");
}
static void book_on_media_ready(app_ctx_t* ctx) {
    if(s_view==CATEGORIES || (s_view==SEARCH&&s_search_parent==CATEGORIES)){
        ui_keyboard_end();category_reset();s_view=SHELF;s_shelf_selecting=false;clear_selection();ctx->leaf=0;
    }
    s_shelf_cache_valid = false;
    if (s_view == SHELF) s_scan_pending = true;
}
// 控件编号仅用于保持按下与抬起命中同一个目标。/ IDs pair a press with release on the same control.
static int control_at(app_ctx_t* ctx, uint16_t x, uint16_t y, EpdRect* rect) {
    if(s_view==SHELF && s_shelf_selecting)return -1;
    if(s_view==SHELF && ((y>=160&&y<192) || (s_category_result[0] && y>=1056&&y<1112))) {
        *rect=y>=1056?(EpdRect){0,1056,684,56}:(EpdRect){0,160,684,32};return y>=1056?731:730;
    }
    if (s_view==SHELF && (book_shelf_categories_menu_is_open(&s_shelf_categories) || ui_rect_hit(shelf_category_title_rect(),x,y))) return -1;
    if (s_view == BULK) return bulk_control_at(ctx, x, y, rect);
    if (s_view == MANAGE) {
        *rect = manage_back_rect();
        if (ui_rect_hit(*rect, x, y)) return 711;
    }
    if (s_view == IMPORT) {
        *rect = (EpdRect){36, 79, 58, 58};
        if (ui_rect_hit(*rect, x, y)) return 700;
        for (int i = 0; i < 4; ++i) {
            *rect = import_rect(i);
            if (ui_rect_hit(*rect, x, y)) return 701 + i;
        }
        int tab = ui_nav_hit(x, y);
        if (tab >= 0) { *rect = (EpdRect){tab * 171, UI_NAV_TOP, 171, UI_LOCK_HEIGHT - UI_NAV_TOP}; return 705 + tab; }
        return -1;
    }
    if (s_view == SEARCH) {
        for (int i = 0; i < 45; ++i) { *rect = search_rect(i); if (ui_rect_hit(*rect, x, y)) return 500 + i; }
        return -1;
    }
    if (s_view == MANAGE) {
        if (!s_clear_confirm && !s_file_removed) {
            *rect = (EpdRect){UI_MARGIN, manage_panel().y + manage_panel().height - 258, ui_content_width(), 58};
            if (ui_rect_hit(*rect, x, y)) return 407;
            *rect = (EpdRect){UI_MARGIN, manage_panel().y + manage_panel().height - 190, ui_content_width(), 68};
            if (ui_rect_hit(*rect, x, y)) return 404;
        }
        int count = s_clear_confirm || s_file_removed ? 2 : 4;
        for (int i = 0; i < count; ++i) {
            *rect = manage_rect(i, count);
            if (ui_rect_hit(*rect, x, y)) return s_clear_confirm ? 300 + i : s_file_removed && i == 1 ? 403 : 400 + i;
        }
        return -1;
    }
    if (s_clear_confirm) {
        for (int i = 0; i < 2; ++i) {
            *rect = ui_row_rect(i, 2, 610, UI_BTN_H);
            if (ui_rect_hit(*rect, x, y)) return 300 + i;
        }
        return -1;
    }
    if (s_view == READING) {
        if (s_reader_panel == READER_PANEL_TOOLS) for (int i = 0; i < BOOK_TOOL_COUNT; ++i) {
            *rect = tool_rect(i);
            if (ui_rect_hit(*rect, x, y)) return 200 + i;
        }
        return -1;
    }
    if (s_view != SHELF) for (int i = 0; i < 3; ++i) {
        *rect = ui_bar_rect(i, 3);
        if (ui_rect_hit(*rect, x, y)) return 100 + i;
    }
    if (s_view == SHELF && ui_rect_hit(shelf_manage_rect(), x, y)) { *rect = shelf_manage_rect(); return 114; }
    if (s_view == SHELF && ui_rect_hit(shelf_import_rect(), x, y)) { *rect = shelf_import_rect(); return 115; }
    if(s_view==SHELF && !s_shelf_selecting)for(int action=2;action<4;++action){
        *rect=shelf_header_action_rect(action);
        if(ui_rect_hit(*rect,x,y))return 118+action;
    }
    if (s_view == SHELF) {
        for (int i = UI_PAGER_FIRST; i <= UI_PAGER_LAST; ++i) {
            *rect = i==UI_PAGER_PREVIOUS?shelf_page_arrow_rect(-1):i==UI_PAGER_NEXT?shelf_page_arrow_rect(1):ui_pager_rect(i);
            if (ui_rect_hit(*rect, x, y)) return 116 + i;
        }
    }
    if (s_message[0] && !((s_view == SHELF || s_view == BULK) && s_visible_count)) return -1;
    int rows = s_view == BULK ? BOOK_BULK_ROWS : shelf_rows();
    int count = s_visible_count;
    for (int row = 0; row < rows && ctx->leaf * rows + row < count; ++row) {
        *rect = row_rect(row);
        if (ui_rect_hit(*rect, x, y)) return row;
    }
    return -1;
}
static app_redraw_t paint_control(app_ctx_t* ctx, EpdRect rect) {
    if(s_view==SHELF)for(int i=UI_PAGER_FIRST;i<=UI_PAGER_LAST;++i){
        EpdRect r=ui_pager_rect(i);
        if(!memcmp(&rect,&r,sizeof(r)))return shelf_key_paint(ctx);
    }
    if (s_view == SHELF && app_settings_shelf_style() == 5) {
        for (int row = 0; row < shelf_rows(); ++row) {
            EpdRect target = row_rect(row);
            if (memcmp(&rect, &target, sizeof(rect))) continue;
            s_area = paint_list_cover_feedback(ctx, row);
            s_mode = app_settings_main_fast_refresh() ? MODE_DU : MODE_GL16;
            s_shelf_feedback_pending = true;
            return APP_REDRAW_AREA;
        }
    }
    // 小按钮反馈只重画按钮，不重新绘制全部封面；其余装饰沿用原绘制路径。
    // Repaint small header buttons alone rather than every cover; other decorations retain their path.
    bool header_painted=false;
    if(s_view==SHELF)for(int action=0;action<(s_shelf_selecting?2:4);++action){
        EpdRect target=action==0?shelf_manage_rect():action==1?shelf_import_rect():shelf_header_action_rect(action);
        if(memcmp(&rect,&target,sizeof(rect)))continue;
        draw_shelf_header_button(ctx->fb,rect,action);header_painted=true;break;
    }
    if(!header_painted)render(ctx,ctx->fb);
    s_mode = MODE_DU;
    s_shelf_feedback_pending = false;
    if (s_view == SHELF) {
        for (int row = 0; row < shelf_rows(); ++row) {
            EpdRect target = row_rect(row);
            if (rect.x != target.x || rect.y != target.y ||
                rect.width != target.width || rect.height != target.height) continue;
            // 覆盖抽出前后的并集，释放或取消时也擦净上方的旧封面。
            // Refresh both book positions so release or cancellation clears the lifted edge.
            rect.y -= SHELF_BOOK_LIFT_PX;
            rect.height += SHELF_BOOK_LIFT_PX;
            s_mode = MODE_GL16;
            s_shelf_feedback_pending = true;
            break;
        }
    }
    s_area = rect;
    return APP_REDRAW_AREA;
}
// 左右滑动始终可用；上下点按模式另外保留上下滑动。
// Horizontal swipes work in both tap modes; vertical mode also keeps vertical swipes.
// 从屏幕底部120px上滑呼出工具栏；其他面板保留自己的滑动，正文滑动仍按翻页模式解释。
// Open tools from the bottom 120px; other panels keep their own swipes and mid-body gestures retain turn semantics.
static bool reader_tools_swipe(app_ctx_t *ctx, const ui_gesture_event_t *ev) {
    if (s_view != READING || !s_text || s_reader_panel != READER_PANEL_NONE ||
        s_clear_confirm || s_scan_pending || ctx->consumed || ev->type != UI_GESTURE_SWIPE_U ||
        ev->x0 >= UI_LOCK_WIDTH || ev->y0 < UI_LOCK_HEIGHT - 120 || ev->y0 >= UI_LOCK_HEIGHT) return false;
    s_pressed_control = s_reader_slider = -1;
    s_reader_panel = READER_PANEL_TOOLS;
    s_reader_tools_toggle = true;
    s_stats_activity_ms = ctx->now_ms;
    ctx->user_activity = true;
    invalidate_prep();
    return true;
}
static int reader_swipe_direction(ui_gesture_type_t type, bool vertical) {
    int direction = 0;
    if (type == UI_GESTURE_SWIPE_L || (vertical && type == UI_GESTURE_SWIPE_U)) direction = 1;
    else if (type == UI_GESTURE_SWIPE_R || (vertical && type == UI_GESTURE_SWIPE_D)) direction = -1;
    if (direction && !vertical && app_settings_book_vertical()) direction = -direction;
    return direction;
}

// 标题样式面板：一次手势走一槽或两槽，落点始终对齐槽边界，卡片不会被切开。
// Heading-style sheet: one gesture moves one or two slots and always lands on a slot boundary, so no
// card is ever cut in half.
static app_redraw_t title_style_scroll_gesture(app_ctx_t *ctx, const ui_gesture_event_t *ev) {
    // 只接管视口内起手的纵向手势；头部返回键与底部按钮保持原语义。
    // Take over only vertical gestures that start inside the viewport; header and footer keep their taps.
    if (ev->y0 < TITLE_STYLE_VIEW_TOP || ev->y0 >= TITLE_STYLE_VIEW_BOTTOM) return APP_REDRAW_NONE;
    if (s_title_style_drag) {
        if (ev->type != UI_GESTURE_MOVE) s_title_style_drag = false;
        return APP_REDRAW_NONE;
    }
    int dy = (int)ev->y - ev->y0, dx = (int)ev->x - ev->x0;
    int distance = dy < 0 ? -dy : dy;
    if (ev->type == UI_GESTURE_MOVE) {
        int across = dx < 0 ? -dx : dx;
        if (distance < 108 || distance <= across) return APP_REDRAW_NONE;
    }
    int steps = distance < 320 ? 1 : 2;
    bool up = ev->type == UI_GESTURE_SWIPE_U || (ev->type == UI_GESTURE_MOVE && dy < 0);
    int content_height = (APP_BOOK_TITLE_STYLE_COUNT + 1) * TITLE_STYLE_CARD_PITCH;
    int max_scroll = content_height - TITLE_STYLE_VIEW_HEIGHT;
    int next = s_title_style_scroll + (up ? steps : -steps) * TITLE_STYLE_CARD_PITCH;
    if (next < 0) next = 0;
    if (next > max_scroll) next = max_scroll;
    if (ev->type == UI_GESTURE_MOVE) s_title_style_drag = true;
    if (next == s_title_style_scroll) return APP_REDRAW_NONE;
    s_title_style_scroll = next;
    render(ctx, ctx->fb);
    s_area = (EpdRect){0, TITLE_STYLE_VIEW_TOP, UI_LOCK_WIDTH, TITLE_STYLE_VIEW_HEIGHT};
    s_mode = MODE_GL16;
    return APP_REDRAW_AREA;
}
// 按键控制列表比视口多一行：纵向手势按整行滚动，落点始终对齐行边界。
// The key-control list outgrows its viewport by one row: a vertical gesture scrolls whole rows and
// always lands on a row boundary.
static app_redraw_t key_settings_scroll_gesture(app_ctx_t *ctx, const ui_gesture_event_t *ev) {
    if (ev->y0 < KEY_ROW_TOP || ev->y0 >= KEY_ROW_TOP + KEY_ROW_VIEW_HEIGHT) return APP_REDRAW_NONE;
    if (s_key_drag) {
        if (ev->type != UI_GESTURE_MOVE) s_key_drag = false;
        return APP_REDRAW_NONE;
    }
    int dy = (int)ev->y - ev->y0, dx = (int)ev->x - ev->x0;
    int distance = dy < 0 ? -dy : dy;
    if (ev->type == UI_GESTURE_MOVE) {
        int across = dx < 0 ? -dx : dx;
        if (distance < 108 || distance <= across) return APP_REDRAW_NONE;
    }
    bool up = ev->type == UI_GESTURE_SWIPE_U || (ev->type == UI_GESTURE_MOVE && dy < 0);
    int next = s_key_scroll + (up ? KEY_ROW_PITCH : -KEY_ROW_PITCH);
    if (next < 0) next = 0;
    if (next > KEY_ROW_MAX_SCROLL) next = KEY_ROW_MAX_SCROLL;
    if (ev->type == UI_GESTURE_MOVE) s_key_drag = true;
    if (next == s_key_scroll) return APP_REDRAW_NONE;
    s_key_scroll = next;
    render(ctx, ctx->fb);
    s_area = (EpdRect){0, KEY_ROW_TOP, UI_LOCK_WIDTH, KEY_ROW_VIEW_HEIGHT};
    s_mode = MODE_GL16;
    return APP_REDRAW_AREA;
}

static app_redraw_t gesture_event(app_ctx_t* ctx, const ui_gesture_event_t* ev) {
    if (s_view == CATEGORIES) return category_gesture(ctx, ev);
    if(s_view==SHELF && s_shelf_selecting) {
        if(ev->type==UI_GESTURE_TAP){
            if(book_shelf_categories_menu_is_open(&s_shelf_categories) || ui_rect_hit(book_selection_rect(BOOK_SELECTION_FILTER),ev->x0,ev->y0))return shelf_category_gesture(ctx,ev);
            return action_at(ctx,ev->x0,ev->y0);
        }
        if(ev->type==UI_GESTURE_SWIPE_L || ev->type==UI_GESTURE_SWIPE_U)return shelf_turn_page(ctx,1);
        if(ev->type==UI_GESTURE_SWIPE_R || ev->type==UI_GESTURE_SWIPE_D)return shelf_turn_page(ctx,-1);
        return APP_REDRAW_NONE;
    }
    bool indent_handled=false;
    app_redraw_t indent_redraw=reader_indent_gesture(ctx,ev,&indent_handled);
    if(indent_handled)return indent_redraw;
    // 正文里的点按或滑动是另一件事，丢掉还没到期的中键单击；取消/按下不算。
    // A body tap or swipe is a different action, so drop an unexpired middle click; cancel/press do not.
    if (s_view == READING && (ev->type == UI_GESTURE_TAP || ev->type == UI_GESTURE_LONG_PRESS ||
        ev->type == UI_GESTURE_SWIPE_L || ev->type == UI_GESTURE_SWIPE_R ||
        ev->type == UI_GESTURE_SWIPE_U || ev->type == UI_GESTURE_SWIPE_D))
        middle_pending_clear();
    if (s_view==SHELF && (book_shelf_categories_menu_is_open(&s_shelf_categories) || ui_rect_hit(shelf_category_title_rect(),ev->x0,ev->y0)))
        return shelf_category_gesture(ctx,ev);
    if (s_view == READING && s_reader_panel == READER_PANEL_TITLE_STYLE) {
        // TAP 留给 action_at 选样式；只在这里接住纵向移动与手势收尾。
        // TAP stays with action_at for picking a style; only vertical motion and gesture ends land here.
        if (ev->type == UI_GESTURE_PRESS) {
            s_pressed_control = -1;
            s_title_style_drag = false;
            return APP_REDRAW_NONE;
        }
        if (ev->type == UI_GESTURE_CANCEL) { s_title_style_drag = false; return APP_REDRAW_NONE; }
        if (ev->type == UI_GESTURE_MOVE || ev->type == UI_GESTURE_SWIPE_U ||
            ev->type == UI_GESTURE_SWIPE_D)
            return title_style_scroll_gesture(ctx, ev);
    }
    if (s_view == READING && s_reader_panel == READER_PANEL_KEY_SETTINGS) {
        // TAP 仍交给 action_at 选行；这里只接管列表视口内的纵向滚动与手势收尾。
        // TAP still picks a row in action_at; only vertical scrolling inside the list viewport
        // and gesture ends land here.
        if (ev->type == UI_GESTURE_PRESS) {
            s_pressed_control = -1;
            s_key_drag = false;
            return APP_REDRAW_NONE;
        }
        if (ev->type == UI_GESTURE_CANCEL) { s_key_drag = false; return APP_REDRAW_NONE; }
        if (ev->type == UI_GESTURE_MOVE || ev->type == UI_GESTURE_SWIPE_U ||
            ev->type == UI_GESTURE_SWIPE_D)
            return key_settings_scroll_gesture(ctx, ev);
    }
    if (s_view == READING && (s_reader_panel == READER_PANEL_CHROME_SETTINGS ||
                              s_reader_panel == READER_PANEL_CHROME_PICKER)) {
        // 与按键控制同构：TAP 留给命中测试，这里只接管列表视口内的纵向滚动。
        // Mirrors the key sheet: TAP stays with hit testing; only vertical scrolling inside the
        // list viewport lands here.
        if (ev->type == UI_GESTURE_PRESS) {
            s_pressed_control = -1;
            s_chrome_drag = s_chrome_pick_drag = false;
            return APP_REDRAW_NONE;
        }
        if (ev->type == UI_GESTURE_CANCEL) {
            s_chrome_drag = s_chrome_pick_drag = false;
            return APP_REDRAW_NONE;
        }
        if (ev->type == UI_GESTURE_MOVE || ev->type == UI_GESTURE_SWIPE_U ||
            ev->type == UI_GESTURE_SWIPE_D) {
            if (s_reader_panel == READER_PANEL_CHROME_SETTINGS)
                return chrome_list_scroll(ctx, ev, CHROME_ROW_TOP, CHROME_ROW_VIEW_HEIGHT,
                                          CHROME_ROW_PITCH, CHROME_ROW_MAX_SCROLL,
                                          &s_chrome_scroll, &s_chrome_drag);
            return chrome_list_scroll(ctx, ev, CHROME_PICK_TOP, CHROME_PICK_VIEW_HEIGHT,
                                      CHROME_PICK_PITCH, CHROME_PICK_MAX_SCROLL,
                                      &s_chrome_pick_scroll, &s_chrome_pick_drag);
        }
    }
    if (reader_tools_swipe(ctx, ev)) return APP_REDRAW_PAGE;
    bool input_feedback = false;
    if (s_view == SEARCH || s_view == EDIT) {
        if (ev->type == UI_GESTURE_PRESS)
            return ui_keyboard_press(ev->x0, ev->y0, 560, ctx->now_ms) ? editor_paint(ctx, false) : APP_REDRAW_NONE;
        input_feedback = ev->type != UI_GESTURE_LONG_PRESS && ui_keyboard_release();
    }
    if (s_view == SEARCH) {
        if (ev->type == UI_GESTURE_TAP) return action_at(ctx, ev->x0, ev->y0);
        if (ev->type == UI_GESTURE_SWIPE_L || ev->type == UI_GESTURE_SWIPE_R) {
            if (!ui_keyboard_page(ev->type == UI_GESTURE_SWIPE_L ? 1 : -1) && !input_feedback) return APP_REDRAW_NONE;
            return editor_paint(ctx, false);
        }
        return input_feedback ? editor_paint(ctx, false) : APP_REDRAW_NONE;
    }
    if (s_view == TOC) {
        if (s_toc_jump_open) {
            if (ev->type == UI_GESTURE_PRESS &&
                book_toc_jump_hit(ev->x0, ev->y0) == BOOK_TOC_JUMP_TRACK) {
                // 按下必须只推局部：PAGE 会让 present_page 立刻取消手势，之后再也收不到 MOVE。
                // A press must push a partial area: PAGE makes present_page cancel the gesture, so no
                // MOVE ever arrives and the slider can only be flicked, never dragged.
                s_toc_jump_drag = true;
                s_toc_jump_percent = book_toc_jump_percent(ev->x0);
                s_toc_jump_push_ms = ctx->now_ms;
                ESP_LOGI(TAG, "jump press x0=%u y0=%u pct=%d", ev->x0, ev->y0, s_toc_jump_percent);
                render(ctx, ctx->fb);
                s_area = (EpdRect){44, 480, 596, 250};
                s_mode = MODE_GL16;
                return APP_REDRAW_AREA;
            }
            if (s_toc_jump_drag && ev->type == UI_GESTURE_MOVE) {
                int next = book_toc_jump_percent(ev->x);
                if (next == s_toc_jump_percent) return APP_REDRAW_NONE;
                s_toc_jump_percent = next;
                // 触摸帧 5ms 一帧、一次刷新上百毫秒：节流到 120ms，松手时再补一次终值。
                // Touch frames arrive every 5ms while a refresh costs hundreds; throttle to 120ms and
                // commit the final value on release.
                if (ctx->now_ms - s_toc_jump_push_ms < 120) return APP_REDRAW_NONE;
                s_toc_jump_push_ms = ctx->now_ms;
                ESP_LOGI(TAG, "jump move x=%u pct=%d", ev->x, next);
                render(ctx, ctx->fb);
                s_area = (EpdRect){44, 480, 596, 250};
                s_mode = MODE_GL16;
                return APP_REDRAW_AREA;
            }
            if (s_toc_jump_drag && ev->type != UI_GESTURE_PRESS) {
                s_toc_jump_drag = false;
                if (ev->type == UI_GESTURE_CANCEL) return APP_REDRAW_NONE;
                // 松手不再按释放点重算：释放坐标与最后一条 MOVE 相差数个百分点，会让屏幕上
                // 看到的数字和实际跳转值分家。这里整页重绘一次，把已跟踪的终值和目标位置预览
                // 一起定下来，用户点「跳转」时用的就是看到的那个数。
                // Do not recompute from the release point: it drifts several percent from the last
                // MOVE, which desynchronised the label from the value actually used. One full redraw
                // commits the tracked value together with the target preview.
                ESP_LOGI(TAG, "jump release pct=%d", s_toc_jump_percent);
                return APP_REDRAW_PAGE;
            }
            if (ev->type == UI_GESTURE_TAP) {
                ESP_LOGI(TAG, "jump tap x0=%u y0=%u hit=%d", ev->x0, ev->y0,
                         book_toc_jump_hit(ev->x0, ev->y0));
                return action_at(ctx, ev->x0, ev->y0);
            }
            return APP_REDRAW_NONE;
        }
        if (ev->type == UI_GESTURE_TAP) {
            int start = book_toc_hit(ev->x0, ev->y0, ctx->leaf, book_navigation_count());
            int end = book_toc_hit(ev->x, ev->y, ctx->leaf, book_navigation_count());
            if (start != -1 && start == end) return action_at(ctx, ev->x0, ev->y0);
        }
        if (ev->type == UI_GESTURE_SWIPE_U || ev->type == UI_GESTURE_SWIPE_D) {
            int next = ctx->leaf + (ev->type == UI_GESTURE_SWIPE_U ? 1 : -1);
            if (next >= 0 && next < leaves()) { ctx->leaf = next; return APP_REDRAW_PAGE; }
        }
        return APP_REDRAW_NONE;
    }
    if (s_view == EDIT) {
        if (ev->type == UI_GESTURE_TAP) return editor_action(ctx, ev->x0, ev->y0);
        if (ev->type == UI_GESTURE_SWIPE_L || ev->type == UI_GESTURE_SWIPE_R)
            return (ui_keyboard_page(ev->type == UI_GESTURE_SWIPE_L ? 1 : -1) || input_feedback) ? editor_paint(ctx, false) : APP_REDRAW_NONE;
        return input_feedback ? editor_paint(ctx, false) : APP_REDRAW_NONE;
    }
    if (s_view == SHELF && ev->type == UI_GESTURE_TAP && ui_nav_hit(ev->x0, ev->y0) >= 0) {
        ui_nav_request(ctx, ui_nav_hit(ev->x0, ev->y0));
        return APP_REDRAW_NONE;
    }
    if (s_view == READING && ev->type != UI_GESTURE_PRESS) s_stats_activity_ms = ctx->now_ms;
    // 触摸全程压制晃动翻页，抬手后留一个短窗口吸收点按与翻页回弹。
    // Suppress shake turns throughout touch; keep a short post-release window to absorb taps and turn rebounds.
    if (s_view == READING) s_shake_touch_ms = ctx->now_ms;
    if (s_view == READING &&
        (s_reader_panel == READER_PANEL_FONT_SETTINGS || s_reader_panel == READER_PANEL_LAYOUT_SETTINGS)) {
        if (ev->type == UI_GESTURE_PRESS) {
            int slider = reader_slider_at(ev->x0, ev->y0);
            if (slider >= 0) {
                reader_slider_begin(slider, ev->x0);
                render(ctx, ctx->fb);
                s_area = reader_slider_refresh_rect(slider);
                s_mode = MODE_GL16;
                return APP_REDRAW_AREA;
            }
        } else if (s_reader_slider >= 0) {
            if (ev->type == UI_GESTURE_MOVE) {
                int slider = s_reader_slider;
                reader_slider_move(ev->x);
                render(ctx, ctx->fb);
                s_area = reader_slider_refresh_rect(slider);
                s_mode = MODE_GL16;
                return APP_REDRAW_AREA;
            }
            if (ev->type == UI_GESTURE_TAP || ev->type == UI_GESTURE_SWIPE_L ||
                ev->type == UI_GESTURE_SWIPE_R || ev->type == UI_GESTURE_SWIPE_U ||
                ev->type == UI_GESTURE_SWIPE_D || ev->type == UI_GESTURE_LONG_PRESS) {
                if (!s_reader_slider_endpoint) reader_slider_move(ev->x);
                return reader_slider_commit(ctx);
            }
            if (ev->type == UI_GESTURE_CANCEL) {
                s_reader_slider = -1;
                return APP_REDRAW_PAGE;
            }
        }
    }
    if (s_view == READING && s_reader_panel == READER_PANEL_FONT_PICKER &&
        (ev->type == UI_GESTURE_SWIPE_U || ev->type == UI_GESTURE_SWIPE_D)) {
        int count = ttf_font_count();
        int pages = count > 0 ? (count + BOOK_FONT_PAGE - 1) / BOOK_FONT_PAGE : 1;
        if (ev->type == UI_GESTURE_SWIPE_U && s_font_page + 1 < pages) ++s_font_page;
        else if (ev->type == UI_GESTURE_SWIPE_D && s_font_page > 0) --s_font_page;
        else return APP_REDRAW_NONE;
        return APP_REDRAW_PAGE;
    }
    EpdRect start_rect = {0}, end_rect = {0};
    int start = control_at(ctx, ev->x0, ev->y0, &start_rect);
    int end = control_at(ctx, ev->x, ev->y, &end_rect);
    if (ev->type == UI_GESTURE_PRESS) {
        s_pressed_control = start;
        return start >= 0 ? paint_control(ctx, start_rect) : APP_REDRAW_NONE;
    }
    bool decorated = s_pressed_control >= 0;
    s_pressed_control = -1;
    if (ev->type == UI_GESTURE_LONG_PRESS && start == end) {
        if (s_view == SHELF && start >= 0 && start < shelf_rows() && !s_clear_confirm) {
            s_managed = s_shelf[ctx->leaf * shelf_rows() + start];
            s_view = MANAGE;
            s_clear_confirm = false;
            s_delete_confirm = s_file_removed = s_managed.removed;
            copy_text(s_manage_message, sizeof(s_manage_message), s_file_removed ? "文件已删除，进度清理失败，请重试" : "");
            return APP_REDRAW_PAGE;
        }
        if (s_view == READING && !s_toolbar && !s_clear_confirm && ui_rect_hit(body_rect(), ev->x0, ev->y0)) {
            save_progress();
            set_reader_view(TOC);
            s_message[0] = 0;
            size_t toc = current_toc_position();
            ctx->leaf = toc == SIZE_MAX ? 0 : (int)(toc / BOOK_TOC_ROWS);
            return APP_REDRAW_PAGE;
        }
    }
    if (ev->type == UI_GESTURE_TAP && !s_scan_pending) {
        if (start >= 0 && start == end) {
            // 列表先完成回弹，才释放封面缓存并开书；没有人为等待或整页预刷新。
            // Settle a list bounce before opening frees artwork, without a delay or whole-page pre-refresh.
            if (decorated && s_view == SHELF && app_settings_shelf_style() == 5 && start < shelf_rows())
                (void)present(ctx, paint_control(ctx, start_rect));
            app_redraw_t result = action_at(ctx, ev->x0, ev->y0);
            if (ctx->request_app || ctx->request_menu || ctx->request_return) return result;
            return result != APP_REDRAW_NONE ? result : paint_control(ctx, start_rect);
        }
        if (start < 0 && end < 0 && s_view == READING && !s_clear_confirm)
            return action_at(ctx, ev->x0, ev->y0);
    }
    if (!s_clear_confirm && !s_scan_pending) {
        if (s_view == READING && (!s_toolbar || (s_reader_panel == READER_PANEL_TOOLS && ev->y0 < 1096)) &&
            ui_rect_hit(body_rect(), ev->x0, ev->y0)) {
            int direction = reader_swipe_direction(ev->type, app_settings_reader_vertical_turn());
            if (direction) { ctx->user_activity = true; return turn_page(ctx, direction); }
        } else if (s_view == SHELF && (ev->type == UI_GESTURE_SWIPE_L || ev->type == UI_GESTURE_SWIPE_R)) {
            app_redraw_t result = shelf_turn_page(ctx, ev->type == UI_GESTURE_SWIPE_L ? 1 : -1);
            if (result != APP_REDRAW_NONE) return result;
        } else if ((s_view == BULK && !s_batch_confirm) &&
                   (ev->type == UI_GESTURE_SWIPE_U || ev->type == UI_GESTURE_SWIPE_D)) {
            return bulk_turn_page(ctx, ev->type == UI_GESTURE_SWIPE_U ? 1 : -1);
        }
    }
    return decorated ? paint_control(ctx, start_rect) : APP_REDRAW_NONE;
}
// 只有 NONE↔TOOLS 是纯开关；从设置卡片切过来会整张换掉，必须保留整页差分。
// Only NONE↔TOOLS is a pure toggle; switching in from a settings sheet replaces a whole
// card and must keep the page-wide differential.
static void toggle_reader_tools_panel(void) {
    const bool pure = s_reader_panel == READER_PANEL_NONE || s_reader_panel == READER_PANEL_TOOLS;
    s_reader_panel = s_reader_panel == READER_PANEL_TOOLS ? READER_PANEL_NONE : READER_PANEL_TOOLS;
    if (pure) s_reader_tools_toggle = true;
}
static app_redraw_t reader_key_action(app_ctx_t *ctx, app_reader_key_action_t action) {
    switch (action) {
        case APP_READER_KEY_PREV: case APP_READER_KEY_NEXT:
            s_stats_activity_ms = ctx->now_ms;
            return turn_page(ctx, action == APP_READER_KEY_PREV ? -1 : 1);
        case APP_READER_KEY_HOME: return reader_return(ctx);
        case APP_READER_KEY_FULLSCREEN: return toggle_reader_fullscreen(ctx);
        case APP_READER_KEY_REFRESH: return reader_manual_refresh(ctx);
        case APP_READER_KEY_FRONTLIGHT: {
            // 前光开关只驱动灯，不重画正文；无前光的板保持不操作。
            // The frontlight toggle only drives the light without repainting the body; boards
            // without one stay a no-op.
            if (!frontlight_ready()) return APP_REDRAW_NONE;
            const bool on = !app_settings_frontlight_on();
            app_settings_set_frontlight_on(on);
            (void)frontlight_apply(on, app_settings_frontlight_brightness(), app_settings_frontlight_cct());
            return APP_REDRAW_NONE;
        }
        case APP_READER_KEY_TOOLS:
            s_reader_slider = -1;
            toggle_reader_tools_panel();
            invalidate_prep();
            return APP_REDRAW_PAGE;
        default: return APP_REDRAW_NONE;
    }
}
// 未绑定双击时单击立即生效；绑定后第一次单击先挂起，第二次落在窗口内才算双击。
// 上一次已超时的单击在本次按键里补发，本次单击重新挂起，输入不丢。
// With no double action a single click fires at once; once bound, the first click is held and
// only a second click inside the window counts as a double. An expired single is replayed on
// the next press while the new click becomes pending, so no input is lost.
static app_redraw_t middle_short_press(app_ctx_t *ctx) {
    app_reader_key_action_t action = app_settings_reader_key_action(1);
    app_reader_key_action_t double_action = app_settings_reader_double_action();
    if (double_action == APP_READER_KEY_NONE) return reader_key_action(ctx, action);
    if (s_middle_pending && ctx->now_ms - s_middle_pending_ms <= READER_MIDDLE_DOUBLE_MS) {
        s_middle_pending = false;
        return reader_key_action(ctx, double_action);
    }
    app_redraw_t replayed = APP_REDRAW_NONE;
    if (s_middle_pending) replayed = reader_key_action(ctx, s_middle_pending_action);
    s_middle_pending = true;
    s_middle_pending_action = action;
    s_middle_pending_ms = ctx->now_ms;
    return replayed;
}
// 窗口到期且没有触摸时补发挂起的单击；手指还按着就顺延到抬手之后。
// Replays the held single once the window expires with no contact; a finger still down
// postpones it until release.
static app_redraw_t middle_pending_tick(app_ctx_t *ctx) {
    if (!s_middle_pending) return APP_REDRAW_NONE;
    if (s_view != READING || (ctx->touch && ctx->touch->touched) ||
        ctx->now_ms - s_middle_pending_ms <= READER_MIDDLE_DOUBLE_MS) return APP_REDRAW_NONE;
    s_middle_pending = false;
    return reader_key_action(ctx, s_middle_pending_action);
}
static app_redraw_t on_key(app_ctx_t* ctx, int key) {
    reader_indent_hold_cancel();
    s_pressed_control = -1;
    if (s_view == CATEGORIES) {
        if (key == UI_KEY_2) { category_reset(); ui_nav_request(ctx, 0); return APP_REDRAW_NONE; }
        return category_back(ctx);
    }
    if (s_view == TOC && s_toc_jump_open) {
        if (key == UI_KEY_2) s_toc_jump_open = s_toc_jump_drag = false;
        else if (key == UI_KEY_1 || key == UI_KEY_3) {
            s_toc_jump_percent += key == UI_KEY_1 ? -5 : 5;
            if (s_toc_jump_percent < 0) s_toc_jump_percent = 0;
            if (s_toc_jump_percent > 100) s_toc_jump_percent = 100;
        }
        return APP_REDRAW_PAGE;
    }
    if (s_view != READING && s_view != TOC) {
        if (key == UI_KEY_2) { ui_nav_request(ctx, 0); return APP_REDRAW_NONE; }
        if (s_view == EDIT) { ui_keyboard_end(); free(s_editor_cover); s_editor_cover = NULL; s_view = MANAGE; return APP_REDRAW_PAGE; }
        if (s_view == SEARCH) { search_finish(ctx, false); return APP_REDRAW_PAGE; }
        if (s_batch_confirm) { s_batch_confirm = false; return APP_REDRAW_PAGE; }
        if (s_view == MANAGE || s_view == BULK || s_view == IMPORT) {
            if (s_view == BULK) return bulk_finish(ctx);
            s_view = SHELF; return APP_REDRAW_PAGE;
        }
        if (s_view == SHELF) { ui_nav_request(ctx, 0); return APP_REDRAW_NONE; }
    }
    // 注释弹层和点按一致：任意键只收起弹层，不顺手翻页或换面板。
    // The note sheet behaves like a tap: any key only dismisses it, without also turning or
    // switching panels.
    if (s_view == READING && s_reader_panel == READER_PANEL_NOTE) {
        s_reader_panel = READER_PANEL_NONE;
        invalidate_prep();
        return APP_REDRAW_PAGE;
    }
    if (s_scan_pending || s_clear_confirm) return APP_REDRAW_NONE;
    if (s_view == READING && (key == UI_KEY_1 || key == UI_KEY_2 || key == UI_KEY_3)) {
        if (key != UI_KEY_2) {
            middle_pending_clear();
            return reader_key_action(ctx, app_settings_reader_key_action(key == UI_KEY_1 ? 0 : 2));
        }
        return middle_short_press(ctx);
    }
    if (key == UI_KEY_2) {
        if (s_view == BULK) s_view = SHELF;
        else if (s_view == SHELF) { ui_nav_request(ctx, 3); return APP_REDRAW_NONE; }
        else if (s_view == READING) {
            s_reader_slider = -1;
            toggle_reader_tools_panel();
            invalidate_prep();
        } else if (s_view == TOC) {
            set_reader_view(s_text ? READING : SHELF);
            s_reader_panel = READER_PANEL_NONE;
        } else {
            ui_nav_request(ctx, 0);
            return APP_REDRAW_NONE;
        }
        return APP_REDRAW_PAGE;
    }
    if (key != UI_KEY_1 && key != UI_KEY_3) return APP_REDRAW_NONE;
    int dir = key == UI_KEY_1 ? -1 : 1;
    if (s_view == READING) { s_stats_activity_ms = ctx->now_ms; return turn_page(ctx, dir); }
    int next = ctx->leaf + dir;
    if (next < 0 || next >= leaves()) return APP_REDRAW_NONE;
    ctx->leaf = next;
    return APP_REDRAW_PAGE;
}
static app_redraw_t on_power_short(app_ctx_t* ctx) {
    if (s_view != READING || !s_text || s_clear_confirm ||
        !app_settings_reader_power_turn()) return APP_REDRAW_NONE;
    s_stats_activity_ms = ctx->now_ms;
    app_redraw_t result = turn_page(ctx, 1);
    return result == APP_REDRAW_NONE ? APP_REDRAW_DONE : result;
}
static app_redraw_t on_key_long(app_ctx_t* ctx, int key) {
    reader_indent_hold_cancel();
    if (key != UI_KEY_2) return APP_REDRAW_NONE;
    // 长按是新的明确手势，吃掉未到期的单击，避免先翻页再回首页。
    // A hold is an explicit new gesture: drop the unexpired click instead of compounding actions.
    middle_pending_clear();
    if (s_view == TOC && s_toc_jump_open) {
        s_toc_jump_open = s_toc_jump_drag = false;
        return APP_REDRAW_PAGE;
    }
    if (s_view != READING && s_view != TOC) { ui_nav_request(ctx, 0); return APP_REDRAW_NONE; }
    if (s_view == READING) {
        return reader_key_action(ctx, app_settings_reader_hold_action());
    }
    s_message[0] = 0;
    set_reader_view(s_text ? READING : SHELF);
    s_reader_panel = READER_PANEL_NONE;
    return APP_REDRAW_PAGE;
}
static bool menu_handle_enabled(app_ctx_t* ctx) {
    (void)ctx;
    // 图书所有视图只使用产品导航，旧菜单不能抢占右侧分页按钮。
    // All book views use product navigation; the legacy menu must never steal the right pager.
    return false;
}
// 只有阅读背景正文需要保护图片灰阶；书架、搜索、输入等视图让快捷栏收起走完整归一。
// Only a background reading body needs picture grays protected; the shelf, search and input
// views let the quick-layer close run the full normalization pass.
static bool normalize_hold_gray(app_ctx_t* ctx) {
    (void)ctx;
    return s_view == READING && s_text && app_settings_book_background_path()[0] != '\0';
}
static app_redraw_t shelf_turn_page(app_ctx_t *ctx, int direction) {
    int next = ctx->leaf + direction;
    if (s_view != SHELF || s_scan_pending || s_clear_confirm || next == ctx->leaf || next < 0 || next >= leaves())
        return APP_REDRAW_NONE;
    ctx->leaf = next;
    shelf_category_remember(ctx);
    book_shelf_categories_menu_close(&s_shelf_categories);
    s_shelf_page_pending = true;
    s_pressed_control = -1;
    prepare_covers(ctx);
    render(ctx, ctx->fb);
    s_area = (EpdRect){0, 176, UI_LOCK_WIDTH, 912};
    s_mode = MODE_GL16;
    return APP_REDRAW_AREA;
}

static app_redraw_t shelf_key_tick(app_ctx_t *ctx) {
    if(s_view!=SHELF || ctx->request_app){
        s_shelf_key_mode=s_shelf_key_paint_pending=false;s_shelf_key_delta=s_shelf_key_pressed=0;
        return APP_REDRAW_NONE;
    }
    if(!s_shelf_key_mode || !s_shelf_key_pressed || ctx->now_ms<s_shelf_key_until || ctx->consumed)
        return APP_REDRAW_NONE;
    int dir=(s_shelf_key_delta>0)-(s_shelf_key_delta<0);
    if(dir)s_shelf_key_delta-=dir;
    s_shelf_key_pressed=0;
    app_redraw_t redraw=dir?shelf_turn_page(ctx,dir):APP_REDRAW_NONE;
    if(s_shelf_key_delta){s_shelf_key_pressed=s_shelf_key_delta<0?-1:1;s_shelf_key_until=ctx->now_ms+35;}
    return redraw==APP_REDRAW_NONE?shelf_key_paint(ctx):redraw;
}

// 键事件与原始边沿可能属于同一份报告，每轮最多执行一次；其他子页只清理旧输入。
// Keys and raw edges may come from the same report: apply one turn per tick, discard on other subpages.
static int book_remote_direction(void) {
    int direction = 0;
    ble_pt_event_t event;
    while (ble_pt_pop_key(&event)) {
        ble_pt_action_t action = ble_pt_action_for_usage(event.usage, event.mods);
        if (action == BLE_PT_ACTION_PREV) direction = -1;
        else if (action == BLE_PT_ACTION_NEXT) direction = 1;
    }
    ble_pt_raw_t raw;
    while (ble_pt_pop_raw(&raw)) {
        if (!raw.pressed) continue;
        ble_pt_action_t action = ble_pt_action_for_frame(&raw.frame);
        if (action == BLE_PT_ACTION_PREV) direction = -1;
        else if (action == BLE_PT_ACTION_NEXT) direction = 1;
    }
    return direction;
}
// 音量键翻页：侧键低电平为按下，只取按下沿，一次按一下翻一页；面板或书架不响应。
// Volume-key turns: the side keys read low when pressed; only the press edge counts, one turn per
// press, and panels or the shelf stay inert.
#define READER_VOLUME_POLL_MS 30
static app_redraw_t reader_volume_tick(app_ctx_t* ctx) {
    static int64_t s_poll_ms;
    static uint8_t s_last_keys = 0xff;
    if (!app_settings_reader_volume_turn() || !app_book_reader_body_visible()) {
        s_last_keys = 0xff;
        return APP_REDRAW_NONE;
    }
    if (ctx->now_ms - s_poll_ms < READER_VOLUME_POLL_MS) return APP_REDRAW_NONE;
    s_poll_ms = ctx->now_ms;
    uint8_t keys = 0;
    if (board_read_volume_keys(&keys) != ESP_OK) {
        s_last_keys = 0xff;
        return APP_REDRAW_NONE;
    }
    uint8_t edges = (uint8_t)(keys & ~s_last_keys);
    s_last_keys = keys;
    if (!edges) return APP_REDRAW_NONE;
    s_stats_activity_ms = ctx->now_ms;
    return turn_page(ctx, (edges & BOARD_VOLUME_UP) ? -1 : 1);
}
// 上键单击：按下沿去抖，松开时仍在短按窗口内才执行；按住不会重复触发。
// Up-key single press: debounce the press edge and fire on release inside the short-press window;
// holding the key never repeats.
#define READER_UP_KEY_DEBOUNCE_MS 30
#define READER_UP_KEY_LONG_MS 1000
static app_redraw_t reader_up_key_tick(app_ctx_t* ctx) {
    static int64_t s_poll_ms;
    static bool s_down;
    static int64_t s_down_ms;
    app_reader_key_action_t action = app_settings_reader_up_action();
    if (action == APP_READER_KEY_NONE || !app_book_reader_body_visible()) {
        s_down = false;
        return APP_REDRAW_NONE;
    }
    if (ctx->now_ms - s_poll_ms < READER_VOLUME_POLL_MS) return APP_REDRAW_NONE;
    s_poll_ms = ctx->now_ms;
    bool pressed = false;
    if (board_read_up_key(&pressed) != ESP_OK) {
        s_down = false;
        return APP_REDRAW_NONE;
    }
    if (pressed) {
        if (!s_down) { s_down = true; s_down_ms = ctx->now_ms; }
        return APP_REDRAW_NONE;
    }
    if (!s_down) return APP_REDRAW_NONE;
    s_down = false;
    int64_t held = ctx->now_ms - s_down_ms;
    if (held < READER_UP_KEY_DEBOUNCE_MS || held >= READER_UP_KEY_LONG_MS) return APP_REDRAW_NONE;
    s_stats_activity_ms = ctx->now_ms;
    return reader_key_action(ctx, action);
}
// 屏轴映射：面板固定倒置竖屏，整机 X（board_accel_to_device 已转换）为屏幕短边横轴，
// Y 为长边、Z 出屏。sx 符号已按真机校准（2026-10-31）：整机 x 与屏幕右向相反，取负号，
// 右晃为下一页；若换装或面板方向变更需重新校准。
// Screen-axis mapping: the panel is fixed inverted portrait; device X (already converted by
// board_accel_to_device) is the screen's short-edge horizontal axis, Y the long edge and Z out
// of the screen. The sx sign is device-calibrated (2026-10-31): device x opposes the screen's
// rightward direction, so it is negated to make a right shake turn forward. Recalibrate if the
// board or panel orientation ever changes.
static void reader_shake_screen_axes(const sc7a20h_sample_t* sample, int* sx, int* sy, int* sz) {
    *sx = -sample->x_mg;
    *sy = sample->y_mg;
    *sz = sample->z_mg;
}
// 晃动翻页：阅读正文横晃一次翻一页，判定在 book_policy.h；触摸 600ms 内与回弹由门槛屏蔽。
// 传感器只在正文可见时上电（25Hz HR 约 10µA），离页、锁屏与重排窗口自动断电；浅睡会被
// 拿起唤醒重配，醒来靠 pending 重放本配置。
// Shake turns: one horizontal shake turns one page in the reader body; the book_policy gate
// rejects the 600ms touch window and rebounds. The sensor powers up only while the body is
// visible (25Hz HR, ~10uA) and sleeps on exit, lock or panel windows; light sleep may leave the
// pickup-wake config behind, so the pending flag replays ours after wake.
// 轮询周期必须不小于传感器出数周期（25Hz 即 40ms）：30ms 会把同一样本读两次，重复样本的
// jerk=0 会被策略当作静止而打开重力跟踪，基准在甩动中途被拖走，门槛从此无法重臂。
// The poll period must not go below the sensor's output period (40ms at 25Hz): polling faster
// feeds the gate the same sample twice, whose zero jerk reads as rest and opens gravity
// tracking mid-swing, dragging the baseline away until nothing re-arms.
#define READER_SHAKE_POLL_MS 50
#define READER_SHAKE_TOUCH_QUIET_MS 600
#define READER_SHAKE_CFG_BACKOFF_MS 2000
static const sc7a20h_sensor_config_t READER_SHAKE_SENSOR_CONFIG = {
    .odr = SC7A20H_ODR_25, .fs = SC7A20H_FS_2G, .mode = SC7A20H_MODE_HR,
    .osr = SC7A20H_OSR_OFF, .dlpf = SC7A20H_DLPF_OFF, .hpf = SC7A20H_HPF_OFF,
    .axis_mask = SC7A20H_AXIS_XYZ,
};
static app_redraw_t reader_shake_tick(app_ctx_t* ctx) {
    static int64_t s_poll_ms, s_cfg_fail_ms, s_sleep_retry_ms;
    if (ctx->touch && ctx->touch->touched) s_shake_touch_ms = ctx->now_ms;
    const bool active = app_settings_reader_shake_turn() &&
                        app_book_reader_body_visible() && ctx->acc != NULL;
    if (!active) {
        if (s_shake_sensor_on && ctx->acc && ctx->now_ms >= s_sleep_retry_ms) {
            board_sensor_sleep(ctx->acc);
            s_shake_sensor_on = sc7a20h_powered(ctx->acc);
            s_sleep_retry_ms = s_shake_sensor_on ? ctx->now_ms + READER_SHAKE_CFG_BACKOFF_MS : 0;
            s_shake_cfg_pending = true;
        }
        return APP_REDRAW_NONE;
    }
    s_sleep_retry_ms = 0;
    if (ctx->now_ms - s_poll_ms < READER_SHAKE_POLL_MS) return APP_REDRAW_NONE;
    s_poll_ms = ctx->now_ms;
    if (!sc7a20h_powered(ctx->acc) || s_shake_cfg_pending) {
        if (ctx->now_ms - s_cfg_fail_ms < READER_SHAKE_CFG_BACKOFF_MS) return APP_REDRAW_NONE;
        // apply_config 本身会置起供电；失败退避 2s 再试，不给坏总线刷 I2C。
        // apply_config powers the sensor up itself; back off 2s on failure instead of
        // hammering a broken bus.
        if (sc7a20h_apply_config(ctx->acc, &READER_SHAKE_SENSOR_CONFIG) != ESP_OK) {
            s_cfg_fail_ms = ctx->now_ms;
            return APP_REDRAW_NONE;
        }
        s_shake_cfg_pending = false;
        s_shake_sensor_on = true;
    }
    sc7a20h_sample_t sample;
    if (board_accel_read(ctx->acc, &sample) != ESP_OK) return APP_REDRAW_NONE;
    int sx, sy, sz;
    reader_shake_screen_axes(&sample, &sx, &sy, &sz);
    int dir = book_shake_feed(&s_shake_gate, sx, sy, sz,
                              ctx->now_ms - s_shake_touch_ms < READER_SHAKE_TOUCH_QUIET_MS,
                              ctx->now_ms);
    if (!dir) return APP_REDRAW_NONE;
    s_stats_activity_ms = ctx->now_ms;
    return turn_page(ctx, dir);
}
static app_redraw_t on_tick(app_ctx_t* ctx) {
    if (s_view == CATEGORIES) return category_tick(ctx);
    app_redraw_t indent=reader_indent_hold_tick(ctx);
    if(indent!=APP_REDRAW_NONE)return indent;
    app_redraw_t keys=shelf_key_tick(ctx);
    if(keys!=APP_REDRAW_NONE || ctx->request_app)return keys;
    // 字体开关留下的重排在这里做：触摸回调的栈太深，一章排下来会把主任务栈顶穿（实测溢出过）。
    // The font switch's re-typeset happens here: the touch callback runs too deep, and laying out a
    // chapter from there overran the main task stack (measured).
    if (s_fonts_reload_pending) {
        s_fonts_reload_pending = false;
        if (s_text && book_kind() == BOOK_KIND_EPUB)
            (void)load_chapter(ctx, s_reader_session.position.chapter, book_layout_page_start_offset(s_reader_session.position.page), false);
        return APP_REDRAW_PAGE;
    }
    TD_TIME("ticket", track_ticket_stats(ctx));
    if (s_view == SEARCH || s_view == EDIT) {
        if (ctx->consumed) return APP_REDRAW_NONE;
        bool held = !ctx->released && ctx->touch && ctx->touch->touched && ctx->touch->count == 1;
        if (ui_keyboard_hold_tick(held, ctx->touch ? ctx->touch->x : 0,
            ctx->touch ? ctx->touch->y : 0, ctx->now_ms) == UI_KEYBOARD_CHANGED) return editor_paint(ctx, false);
        return ui_keyboard_idle_tick(ctx->touch && ctx->touch->touched, ctx->now_ms) == UI_KEYBOARD_CHANGED
            ? editor_paint(ctx, false) : APP_REDRAW_NONE;
    }
    if (ctx->consumed) return APP_REDRAW_NONE;
    app_redraw_t middle = middle_pending_tick(ctx);
    if (middle != APP_REDRAW_NONE) return middle;
    // 上/左为上一页，下/右为下一页；书架复用滑动的局部刷新，不闪动题头和底栏。
    // Up/left turn back, down/right turn forward; the shelf reuses swipe refresh without flashing chrome.
    if (app_settings_ble_turner()) {
        int ble_dir = book_remote_direction();
        if (ble_dir && s_view == SHELF) {
            app_redraw_t redraw = shelf_turn_page(ctx, ble_dir);
            if (redraw != APP_REDRAW_NONE) return redraw;
        } else if (ble_dir && s_view == READING) {
            s_stats_activity_ms = ctx->now_ms;
            return turn_page(ctx, ble_dir);
        }
    }
    app_redraw_t volume = reader_volume_tick(ctx);
    if (volume != APP_REDRAW_NONE) return volume;
    app_redraw_t up_key = reader_up_key_tick(ctx);
    if (up_key != APP_REDRAW_NONE) return up_key;
    app_redraw_t shake = reader_shake_tick(ctx);
    if (shake != APP_REDRAW_NONE) return shake;
    if (s_reader_notice[0] && ctx->now_ms >= s_reader_notice_until) {
        s_reader_notice[0] = 0;
        if (s_view == READING) {
            // 区域刷新不会自动重绘帧缓冲；先移除提示，再只更新提示所在区域。
            // Area refreshes do not rerender the framebuffer; erase the toast before updating its region.
            render(ctx, ctx->fb);
            s_area = (EpdRect){208, 150, 268, 68};
            s_mode = MODE_GL16;
            return APP_REDRAW_AREA;
        }
    }
    if (s_save_failed && ctx->now_ms - s_save_retry_ms >= 15000) {
        s_save_retry_ms = ctx->now_ms;
        bool failed = s_save_failed;
        retry_progress();
        if (failed != s_save_failed) { invalidate_prep(); return APP_REDRAW_PAGE; }
    }
    if (s_scan_pending && ctx->now_ms - s_poll_ms >= 500) {
        s_poll_ms = ctx->now_ms;
        board_sd_info_t info = {0};
        if (board_sd_get_info(&info) == ESP_ERR_NOT_FINISHED) return APP_REDRAW_NONE;
        s_scan_pending = false;
        scan_shelf(ctx);
        if (ctx->request_app) return APP_REDRAW_NONE;
        (void)open_requested_book(ctx);
        return APP_REDRAW_PAGE;
    }
    if ((s_view == SHELF || s_view == MANAGE) && !(ctx->touch && ctx->touch->touched)) {
        (void)prepare_cover_batch(ctx, false);
        // 整页准备好再合并推屏；占位页或失败输出不能消费已加载封面的待刷标记。
        // Reveal only a complete cover group; placeholders and failed output cannot consume pending artwork.
        unsigned changed=shelf_cover_reveal_mask();
        if (ctx->request_app) return APP_REDRAW_NONE;
        EpdRect area = {0};
        for (int row = 0; row < shelf_rows(); ++row) {
            if (!(changed & (1u << row))) continue;
            EpdRect r = row_rect(row);
            if (!area.width) area = r;
            else {
                int right = area.x+area.width > r.x+r.width ? area.x+area.width : r.x+r.width;
                int bottom = area.y+area.height > r.y+r.height ? area.y+area.height : r.y+r.height;
                if (r.x < area.x) area.x = r.x;
                if (r.y < area.y) area.y = r.y;
                area.width = right-area.x; area.height = bottom-area.y;
            }
        }
        if (area.width) {
            render(ctx, ctx->fb);
            s_area = area;
            s_mode = s_view == SHELF && app_settings_main_fast_refresh() ? MODE_DU : MODE_GL16;
            s_shelf_feedback_pending = s_view == SHELF;
            s_cover_batch_present_pending = true;
            book_shelf_covers_presenting(&s_shelf_covers, changed);
            return APP_REDRAW_AREA;
        }
    }
    warm_shelf_lock_cover(ctx);
    if (ctx->request_app) return APP_REDRAW_NONE;
    // 目录暂借系统字体，不应把正文按系统字体重新分页。/ The directory borrows the system face without repaginating the reader.
    if (s_view == READING && s_text && strcmp(s_font_path, ttf_font_path())) {
        size_t off = book_layout_page_start_offset(s_reader_session.position.page);
        save_progress();
        lock_draw();
        invalidate_prep();
        bool ok = reader_session_build_layout(&s_reader_session, reader_session_content(&s_reader_session), body_rect(), s_px);
        if (ok) s_reader_session.position.page = book_layout_page_for_offset(off);
        copy_text(s_font_path, sizeof(s_font_path), ttf_font_path());
        unlock_draw();
        if (!ok) { free_book(); s_view = SHELF; copy_text(s_message, sizeof(s_message), "字体重排失败，请重新打开图书"); }
        return APP_REDRAW_PAGE;
    }
    reader_prefetch_idle(ctx);
    return APP_REDRAW_NONE;
}
static void before_lock(app_ctx_t* ctx) {
    // 锁屏期间不补发按键动作，唤醒后从干净状态开始。
    // Never replay a key action across lock; resume from a clean state.
    middle_pending_clear();
    if(s_view==CATEGORIES || (s_view==SEARCH&&s_search_parent==CATEGORIES) || s_shelf_selecting){
        ui_keyboard_end();book_category_ui_suspend(&s_category_ui);category_reset();s_view=SHELF;s_shelf_selecting=false;clear_selection();sort_shelf(ctx);shelf_category_restore(ctx);
    }
    reader_indent_hold_cancel();
    // 浅睡会给加速度计配拿起唤醒，醒来由晃动采样重放阅读配置。
    // Light sleep rearms the accelerometer for pickup wake; the shake sampler replays the reading config after wake.
    s_shake_cfg_pending = true;
    s_shelf_key_mode=s_shelf_key_paint_pending=false;s_shelf_key_delta=s_shelf_key_pressed=0;
    shelf_category_remember(ctx);
    book_shelf_categories_menu_close(&s_shelf_categories);
    track_ticket_stats(ctx);
    save_progress();
    // 锁屏前丢弃分行预绘，后续只按当前字体重建。/ Discard idle prefetch before lock; rebuild only with the current face.
    lock_draw();
    invalidate_prep();
    unlock_draw();
    // 锁屏票根始终以当前正在阅读的书为准；不要依赖书架最近项是否已经同步。
    if (s_text && s_reader_session.book.path[0]) book_progress_set_last_path(s_reader_session.book.path);
    flush_ticket_stats();
    (void)app_reading_journal_flush();
    s_stats_last_ms = 0;
}
static display_rect_t area_hint(app_ctx_t* ctx) { (void)ctx; return display_rect_from_epd(s_area); }
static bool main_page_visible(app_ctx_t* ctx) {
    (void)ctx;
    return s_view == SHELF && !s_shelf_selecting && !s_clear_confirm && !s_batch_confirm;
}

const app_desc_t app_book = {
    .title = "图书 Books", .detail = "TF 卡 txt / epub 阅读", .enter_full = false, .owns_keys = true,
    .defer_middle_short = true,
    .menu_handle_enabled = menu_handle_enabled,
    .main_page_visible = main_page_visible,
    .normalize_hold_gray = normalize_hold_gray,
    .render = render, .present = present, .on_enter = on_enter, .on_exit = book_on_exit,
    .on_media_lost = book_on_media_lost, .on_media_ready = book_on_media_ready,
    .on_before_lock = before_lock, .on_main_key = shelf_main_key,
    .on_gesture = gesture_event, .on_key = on_key, .on_key_long = on_key_long,
    .on_power_short = on_power_short,
    .on_tick = on_tick, .area_hint = area_hint,
};
