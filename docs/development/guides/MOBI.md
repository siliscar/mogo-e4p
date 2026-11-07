# MOBI 阅读契约 / MOBI Reader Contract

## 格式与入口 / Formats And Entry Points

仅接收 `.mobi`，内容还须通过 PDB `BOOKMOBI`、MOBI6 普通电子书、无加密校验。支持 compression 1/2/17480（未压缩/PalmDOC/HUFF-CDIC）、encoding 65001/1252（UTF-8/Windows-1252）。混合 MOBI6+KF8 固定选 record 0 的 MOBI6；拒绝纯 KF8、词典和非普通电子书。AZW3、PRC、KFX 不在文件入口白名单内。
Only `.mobi` enters the reader, followed by PDB/BOOKMOBI, ordinary MOBI6 and no-encryption checks. Compression 1/2/17480 (uncompressed/PalmDOC/HUFF-CDIC) and encoding 65001/1252 are supported. Hybrid books always select record 0's MOBI6. Pure KF8 and non-ordinary/dictionary books are rejected; AZW3/PRC/KFX extensions are not accepted.

保留既有上传名称限制：网页书籍快捷入口最多 120 个 UTF-8 字节，较长原名可用「添加其他文件」或 USB/TF 卡拷入；通用上传仍受完整相对路径限制，不自动改名或截断。本轮样书原名为 219 字节，可走通用入口。
Existing upload-name limits remain: the typed book shortcut allows 120 UTF-8 bytes. Longer original names can use the general-file action or USB/SD copy, subject to full relative-path limits, without automatic renaming/truncation. The supplied sample's 219-byte name fits the general route.

`book_source` 拥有当前书源，`book_mobi` 只读记录、解压及索引，`book_mobi_markup` 将原编码 HTML 转成共享 `html_text` 块；复用会话、排版、预取、进度、书签和媒体交还。`book_metadata` 为书架、首页及锁屏分派轻量元数据/封面请求，独立句柄不替换正在阅读的书；缓存专用调用不退回解析原书。JPEG/PNG 资源以内部 `mobi-N` 标识访问，不将书内字符串当作文件路径。
`book_source` owns the active source; `book_mobi` owns read-only records, decoding and indices; `book_mobi_markup` feeds shared `html_text`. Sessions, layout, prefetch, progress, bookmarks and media handoff remain shared. `book_metadata` dispatches independent lightweight metadata/cover requests without replacing the active book; cached-only requests never fall back to source parsing. JPEG/PNG resources use internal `mobi-N` identifiers, not filesystem paths.

## 坐标与缓存 / Coordinates And Caches

MOBI v1 的进度以选定 MOBI6 分支的原编码 HTML 字节坐标为基础；读取时按 `mbp:pagebreak` 或达到目标大小后的闭段标签切分。目录优先读 `ncx` 编译出的 INDX 索引（与手机阅读器同一份章节名），缺少至少两项时回退 guide 指向的 TOC 中 `filepos`，再回退 h1-h6 与分段列表。编号目录有至少两项时优先编号正文项；其他内容仍在完整阅读流中。标签内或字符/实体中间的目标向前对齐，但锚点 ID 保持原字节坐标。显示文字、解码后 UTF-8 偏移与原字节坐标不得混用。
MOBI v1 progress is based on the selected MOBI6 branch's original encoded HTML bytes. Segments end at page breaks or safe closing blocks after the target size. Navigation uses the INDX index built from `ncx` first (the same names phone readers show), then authored guide/TOC filepos entries, then h1-h6 and segments. A numbered TOC with at least two body entries takes precedence without removing any readable content. Targets inside tags, characters or entities align backward while retaining raw-coordinate IDs. Raw bytes and normalized UTF-8 offsets are distinct.

TF 卡 `.readpico/index/` 下 `mobi` 与 `mobi-meta` 为可丢弃缓存，按路径、大小、修改时间失效；正文索引另核对 PDB/record 0 指纹、长度、单调坐标和校验和。损坏、截断、不匹配、无法创建或内存不足时丢弃索引并重建，写失败不阻止阅读；原书从不改写。冷/热打开必须产生相同分段、目录、正文及定位。缓存不是原书备份，也不是原书内容完整性证明。
Disposable `mobi` and `mobi-meta` SD caches use path/size/mtime identity. The body index also validates the PDB/record-0 fingerprint, lengths, monotonic coordinates and checksum. Invalid/unavailable caches rebuild; cache-write failure never prevents reading or modifies the source. Cold/hot content and coordinates must agree. Caches are neither backups nor proof of whole-source integrity.

变更分支选择、分段或文本规范化规则前，须设计已有 NVS 进度和书签的迁移/失效策略并测试；只升级缓存版本不足以保证续读语义。现有 TXT/EPUB 坐标与存储协议不变。
Changing branch selection, segmentation or normalization requires an explicit migration/invalidation strategy for existing NVS progress and bookmarks. A cache-version bump alone is insufficient. TXT/EPUB coordinates and persistence stay unchanged.

## 资源与限制 / Resources And Limits

最多 16,384 条 PDB 记录，单记录输入/解压结果最多 64 KiB，选定正文最多 64 MiB，元数据头最多 1 MiB；HUFF-CDIC 码表 2 KiB、词典按 CDIC 记录整条驻留 PSRAM。分段目标 96 KiB、硬上限 256 KiB，分段和目录各最多 2,048 项。记录解压与编码校验流式完成，UTF-8 可跨记录；设备索引每 16 条记录让出调度，整体超过 30 秒报失败。该时限不保证 UI 中途取消或 TF 硬件故障恢复的时延。
Limits: 16,384 PDB records, 64 KiB encoded/decoded per record, 64 MiB selected text and 1 MiB metadata header; the HUFF table is 2 KiB and CDIC records stay resident in PSRAM. Segment target/hard limits are 96/256 KiB, with at most 2,048 segments and navigation entries. Streaming decoding validates UTF-8 across records. Device indexing yields every 16 records and fails after 30 seconds; this does not guarantee mid-open UI cancellation or physical-media fault latency.

仅使用系统/用户字体；书内字体、跨分段脚注弹层及复杂 CSS/固定版式不实现。同分段数字标记复用现有注释弹层，所有脚注正文继续保留。JPEG/PNG 读取最多 4 MiB，受调用方预算及 PSRAM 预留共同限制，解码仍由公共 image 层限制。所有临时分配失败和文件错误交由现有开书/图片失败路径处理，不更改硬件、刷屏或媒体所有权。
Only system/user fonts are used. Embedded fonts, cross-segment note popups and complex CSS/fixed layout are not implemented. Same-segment numeric markers reuse the note sheet; note bodies remain readable. JPEG/PNG reads are capped at 4 MiB, caller budget and PSRAM reserve; shared image decoding adds its own bounds. Allocation/I/O failures follow existing reader/image failure paths without changing hardware, presentation or media ownership.

## 验证与许可 / Verification And Licensing

`python3 tools/test_book_mobi.py` 生成自有夹具，覆盖压缩、编码、混合分支、目录/脚注坐标、封面与插图、冷/热及坏缓存、异常文件和资源边界；`--sample /absolute/path/book.mobi` 仅本机只读检查用户提供的《窄门》样书及已核对的 MOBI6 原始正文哈希。商业样书不复制入仓库或上传。通用回归入口为 `python3 tools/run_reader_baseline.py`，混合开关书和失败注入位于 `source_memory`。宿主/编译结果不代替实屏、开书耗时、TF 拔插/断电及 USB/无线交还验收。
The native test generates original fixtures covering codecs, encodings, hybrid selection, navigation/note coordinates, covers/images, caches, malformed files and limits. Optional `--sample` locally reads the supplied sample and its verified MOBI6 raw hash; commercial content is never copied into Git or uploaded. The baseline runner includes `mobi`; `source_memory` exercises mixed opens/closes and allocation faults. Host/build checks do not establish panel, latency, physical SD/power-loss or USB/radio acceptance.

PalmDOC 解码为固定版本 libmobi 的 LGPL-3.0-or-later 有界子集，来源和改动见 [组件说明](../../../components/mobi_codec/README.md)。正式分发静态链接固件前，须连同相应源码、许可及可重新链接所需材料满足 LGPL 第 4 节；不能用原创内容的个人学习条款限制第三方许可授予的权利。
PalmDOC is a pinned LGPL-3.0-or-later libmobi subset; see the component notes. Before distributing statically linked firmware, provide corresponding source, notices and relinking materials required by LGPL section 4. Original-material personal-study terms cannot restrict third-party grants.
