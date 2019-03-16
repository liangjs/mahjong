/****************************************************************************
 Copyright (c) 2016-2018 Jeff Wang <summer_insects@163.com>

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 ****************************************************************************/

#ifndef MAHJONG_H
#define MAHJONG_H

#include <bits/stdc++.h>

#ifdef _MSC_VER  // for MSVC
#define forceinline __forceinline
#elif defined __GNUC__  // for gcc on Linux/Apple OS X
#define forceinline __inline__ __attribute__((always_inline))
#else
#define forceinline inline
#endif

namespace mahjong {

/**
 * @brief 代码注释中用到的术语简介
 * - 顺子：数牌中，花色相同序数相连的3张牌。
 * - 刻子：三张相同的牌。碰出的为明刻，未碰出的为暗刻。俗称坎。杠也算刻子，明杠算明刻，暗杠算暗刻。
 * - 面子：顺子和刻子的统称。俗称一句话、一坎牌。
 * - 雀头：基本和牌形式中，单独组合的对子，也叫将、眼。
 * - 基本和型：4面子1雀头的和牌形式。
 * - 特殊和型：非4面子1雀头的和牌形式，在国标规则中，有七对、十三幺、全不靠等特殊和型。
 * - 门清：也叫门前清，指不吃、不碰、不明杠的状态。特殊和型必然是门清状态。暗杠虽然不破门清，但会暴露出手牌不是特殊和型的信息。
 * - 副露：吃牌、碰牌、杠牌的统称，即利用其他选手打出的牌完成自己手牌面子的行为，一般不包括暗杠，也叫鸣牌，俗称动牌。
 *     副露有时候也包括暗杠，此时将暗杠称为之暗副露，而吃、碰、明杠称为明副露。
 * - 立牌：整个手牌除去吃、碰、杠之后的牌。
 * - 手牌：包括立牌和吃、碰、杠的牌，有时仅指立牌。
 * - 听牌：只差所需要的一张牌即能和牌的状态。俗称下叫、落叫、叫和（糊）。
 * - 一上听：指差一张就能听牌的状态，也叫一向听、一入听。以此类推有二上听、三上听、N上听。
 * - 上听数：达到听牌状态需要牌的张数。
 * - 有效牌：能使上听数减少的牌，也称进张牌、上张牌。
 * - 改良牌：能使有效牌增加的牌。通俗来说就是能使进张面变宽的牌。
 * - 对子：两张相同的牌。雀头一定是对子，但对子不一定是雀头。
 * - 两面：数牌中，花色相同数字相邻的两张牌，如45m，与两侧的牌都构成顺子。也叫两头。
 * - 嵌张：数牌中，花色相同数字相隔1的两张牌，如57s，只能与中间的牌构成顺子，中间的这张牌称为嵌张。
 * - 边张：也是数字相邻的两张牌，但由于处在边界位置，只能与一侧的牌能构成顺子，如12只能与3构成顺子、89只能与7构成顺子，这张3或者7便称为边张。
 * - 搭子：指差一张牌就能构成1组面子的两张牌。其形态有刻子搭子（即对子）、两面搭子、嵌张搭子、边张搭子。
 * - 复合搭子：多张牌构成的搭子。常见的有：连嵌张、两面带对子、嵌张带对子、边张带对子等等形态。
 * - 对倒：听牌时，其他牌都已经构成面子，剩余两对，只需任意一对成刻即可和牌，此时另一对充当雀头，这种听牌形态叫对倒，也叫双碰、对碰、对杵。
 */


/**
 * @addtogroup tile
 * @{
 */

/**
 * @brief 花色
 */
typedef uint8_t suit_t;

/**
 * @brief 点数
 */
typedef uint8_t rank_t;

#define TILE_SUIT_NONE          0  ///< 无效
#define TILE_SUIT_CHARACTERS    1  ///< 万子（CHARACTERS）
#define TILE_SUIT_BAMBOO        2  ///< 条子（BAMBOO）
#define TILE_SUIT_DOTS          3  ///< 饼子（DOTS）
#define TILE_SUIT_HONORS        4  ///< 字牌（HONORS）

/**
 * @brief 牌\n
 * 内存结构：
 * - 0-3 4bit 牌的点数
 * - 4-7 4bit 牌的花色
 * 合法的牌为：
 * - 0x11 - 0x19 万子（CHARACTERS）
 * - 0x21 - 0x29 条子（BAMBOO）
 * - 0x31 - 0x39 饼子（DOTS）
 * - 0x41 - 0x47 字牌（HONORS）
 */
typedef uint8_t tile_t;

/**
 * @brief 生成一张牌
 *  函数不检查输入的合法性。如果输入不合法的值，将无法保证合法返回值的合法性
 * @param [in] suit 花色
 * @param [in] rank 点数
 * @return tile_t 牌
 */
static forceinline tile_t make_tile(suit_t suit, rank_t rank) {
    return (((suit & 0xF) << 4) | (rank & 0xF));
}

/**
 * @brief 获取牌的花色
 *  函数不检查输入的合法性。如果输入不合法的值，将无法保证合法返回值的合法性
 * @param [in] tile 牌
 * @return suit_t 花色
 */
static forceinline suit_t tile_get_suit(tile_t tile) {
    return ((tile >> 4) & 0xF);
}

/**
 * @brief 获取牌的点数
 *  函数不检查输入的合法性。如果输入不合法的值，将无法保证合法返回值的合法性
 * @param [in] tile 牌
 * @return rank_t 点数
 */
static forceinline rank_t tile_get_rank(tile_t tile) {
    return (tile & 0xF);
}

/**
 * @brief 所有牌的值
 */
enum tile_value_t {
    TILE_1m = 0x11, TILE_2m, TILE_3m, TILE_4m, TILE_5m, TILE_6m, TILE_7m, TILE_8m, TILE_9m,
    TILE_1s = 0x21, TILE_2s, TILE_3s, TILE_4s, TILE_5s, TILE_6s, TILE_7s, TILE_8s, TILE_9s,
    TILE_1p = 0x31, TILE_2p, TILE_3p, TILE_4p, TILE_5p, TILE_6p, TILE_7p, TILE_8p, TILE_9p,
    TILE_E  = 0x41, TILE_S , TILE_W , TILE_N , TILE_C , TILE_F , TILE_P ,
    TILE_TABLE_SIZE
};

/**
 * @brief 所有合法的牌
 */
static const tile_t all_tiles[] = {
    TILE_1m, TILE_2m, TILE_3m, TILE_4m, TILE_5m, TILE_6m, TILE_7m, TILE_8m, TILE_9m,
    TILE_1s, TILE_2s, TILE_3s, TILE_4s, TILE_5s, TILE_6s, TILE_7s, TILE_8s, TILE_9s,
    TILE_1p, TILE_2p, TILE_3p, TILE_4p, TILE_5p, TILE_6p, TILE_7p, TILE_8p, TILE_9p,
    TILE_E , TILE_S , TILE_W , TILE_N , TILE_C , TILE_F , TILE_P
};

/**
 * @brief 牌表类型
 *
 * 说明：在判断听牌、计算上听数等算法中，主流的对于牌有两种存储方式：
 * - 一种是用牌表，各索引表示各种牌拥有的枚数，这种存储方式的优点是在递归计算时削减面子只需要修改表中相应下标的值，缺点是一手牌的总数不方便确定
 * - 另一种是直接用牌的数组，这种存储方式的优点是很容易确定一手牌的总数，缺点是在递归计算时削减面子不方便，需要进行数组删除元素操作
 */
typedef uint16_t tile_table_t[TILE_TABLE_SIZE];

#define PACK_TYPE_NONE 0  ///< 无效
#define PACK_TYPE_CHOW 1  ///< 顺子
#define PACK_TYPE_PUNG 2  ///< 刻子
#define PACK_TYPE_KONG 3  ///< 杠
#define PACK_TYPE_PAIR 4  ///< 雀头

/**
 * @brief 牌组
 *  用于表示一组面子或者雀头
 *
 * 内存结构：
 * - 0-7 8bit tile 牌（对于顺子，则表示中间那张牌，比如234p，那么牌为3p）
 * - 8-11 4bit type 牌组类型，使用PACK_TYPE_xxx宏
 * - 12-15 4bit offer 供牌信息，取值范围为0123\n
 *       0表示暗手（暗顺、暗刻、暗杠），非0表示明手（明顺、明刻、明杠）
 *
 *       对于牌组是刻子和杠时，123分别来表示是上家/对家/下家供的\n
 *       对于牌组为顺子时，由于吃牌只能是上家供，这里用123分别来表示第几张是上家供的
 */
typedef uint16_t pack_t;

/**
 * @brief 生成一个牌组
 *  函数不检查输入的合法性。如果输入不合法的值，将无法保证合法返回值的合法性
 * @param [in] offer 供牌信息
 * @param [in] type 牌组类型
 * @param [in] tile 牌（对于顺子，为中间那张牌）
 */
static forceinline pack_t make_pack(uint8_t offer, uint8_t type, tile_t tile) {
    return (offer << 12 | (type << 8) | tile);
}

/**
 * @brief 牌组是否为明的
 *  函数不检查输入的合法性。如果输入不合法的值，将无法保证合法返回值的合法性
 * @param [in] pack 牌组
 * @return bool
 */
static forceinline bool is_pack_melded(pack_t pack) {
    return !!((pack >> 12) & 0xF);
}

/**
 * @brief 牌组的供牌信息
 *  函数不检查输入的合法性。如果输入不合法的值，将无法保证合法返回值的合法性
 * @param [in] pack 牌组
 * @return uint8_t
 */
static forceinline uint8_t pack_get_offer(pack_t pack) {
    return ((pack >> 12) & 0xF);
}

/**
 * @brief 获取牌组的类型
 *  函数不检查输入的合法性。如果输入不合法的值，将无法保证合法返回值的合法性
 * @param [in] pack 牌组
 * @return uint8_t 牌组类型
 */
static forceinline uint8_t pack_get_type(pack_t pack) {
    return ((pack >> 8) & 0xF);
}

/**
 * @brief 获取牌的点数
 *  函数不检查输入的合法性。如果输入不合法的值，将无法保证合法返回值的合法性
 * @param [in] pack 牌组
 * @return tile_t 牌（对于顺子，为中间那张牌）
 */
static forceinline tile_t pack_get_tile(pack_t pack) {
    return (pack & 0xFF);
}

/**
 * @brief 手牌结构
 *  手牌结构一定满足等式：3*副露的牌组数+立牌数=13
 */
struct hand_tiles_t {
    pack_t fixed_packs[5];      ///< 副露的牌组（面子），包括暗杠
    intptr_t pack_count;        ///< 副露的牌组（面子）数，包括暗杠
    tile_t standing_tiles[13];  ///< 立牌
    intptr_t tile_count;        ///< 立牌数
};


/**
 * @brief 判断是否为绿一色构成牌
 *  函数不检查输入的合法性。如果输入不合法的值，将无法保证合法返回值的合法性
 * @param [in] tile 牌
 * @return bool
 */
static forceinline bool is_green(tile_t tile) {
    // 最基本的逐个判断，23468s及发财为绿一色构成牌
    //return (tile == TILE_2s || tile == TILE_3s || tile == TILE_4s || tile == TILE_6s || tile == TILE_8s || tile == TILE_F);

    // 算法原理：
    // 0x48-0x11=0x37=55刚好在一个64位整型的范围内，
    // 用uint64_t的每一位表示一张牌的标记，事先得到一个魔数，
    // 然后每次测试相应位即可
    return !!(0x0020000000AE0000ULL & (1ULL << (tile - TILE_1m)));
}

/**
 * @brief 判断是否为推不倒构成牌
 *  函数不检查输入的合法性。如果输入不合法的值，将无法保证合法返回值的合法性
 * @param [in] tile 牌
 * @return bool
 */
static forceinline bool is_reversible(tile_t tile) {
    // 最基本的逐个判断：245689s、1234589p及白板为推不倒构成牌
    //return (tile == TILE_2s || tile == TILE_4s || tile == TILE_5s || tile == TILE_6s || tile == TILE_8s || tile == TILE_9s ||
    //    tile == TILE_1p || tile == TILE_2p || tile == TILE_3p || tile == TILE_4p || tile == TILE_5p || tile == TILE_8p || tile == TILE_9p ||
    //    tile == TILE_P);

    // 算法原理同绿一色构成牌判断函数
    return !!(0x0040019F01BA0000ULL & (1ULL << (tile - TILE_1m)));
}

/**
 * @brief 判断是否为数牌幺九（老头牌）
 *  函数不检查输入的合法性。如果输入不合法的值，将无法保证合法返回值的合法性
 * @param [in] tile 牌
 * @return bool
 */
static forceinline bool is_terminal(tile_t tile) {
    // 最基本的逐个判断
    //return (tile == TILE_1m || tile == TILE_9m || tile == TILE_1s || tile == TILE_9s || tile == TILE_1p || tile == TILE_9p);

    // 算法原理：观察数牌幺九的二进制位：
    // 0x11：0001 0001
    // 0x19：0001 1001
    // 0x21：0010 0001
    // 0x29：0010 1001
    // 0x31：0011 0001
    // 0x39：0011 1001
    // 所有牌的低4bit只会出现在0001到1001之间，跟0111位与，只有0001和1001的结果为1
    // 所有数牌的高4bit只会出现在0001到0011之间，跟1100位与，必然为0
    // 于是构造魔数0xC7（1100 0111）跟牌位与，结果为1的，就为数牌幺九
    // 缺陷：低4bit的操作会对0xB、0xD、0xF产生误判，高4bit的操作会对0x01和0x09产生误判
    return ((tile & 0xC7) == 1);
}

/**
 * @brief 判断是否为风牌
 * @param [in] tile 牌
 * @return bool
 */
static forceinline bool is_winds(tile_t tile) {
    return (tile > 0x40 && tile < 0x45);
}

/**
 * @brief 判断是否为箭牌（三元牌）
 * @param [in] tile 牌
 * @return bool
 */
static forceinline bool is_dragons(tile_t tile) {
    return (tile > 0x44 && tile < 0x48);
}

/**
 * @brief 判断是否为字牌
 * @param [in] tile 牌
 * @return bool
 */
static forceinline bool is_honor(tile_t tile) {
    return (tile > 0x40 && tile < 0x48);
}

/**
 * @brief 判断是否为数牌
 * @param [in] tile 牌
 * @return bool
 */
static forceinline bool is_numbered_suit(tile_t tile) {
    if (tile < 0x1A) return (tile > 0x10);
    if (tile < 0x2A) return (tile > 0x20);
    if (tile < 0x3A) return (tile > 0x30);
    return false;
}

/**
 * @brief 判断是否为数牌（更快）
 *  函数不检查输入的合法性。如果输入不合法的值，将无法保证合法返回值的合法性
 * @see is_numbered_suit
 * @param [in] tile 牌
 * @return bool
 */
static forceinline bool is_numbered_suit_quick(tile_t tile) {
    // 算法原理：数牌为0x11-0x19，0x21-0x29，0x31-0x39，跟0xC0位与，结果为0
    return !(tile & 0xC0);
}

/**
 * @brief 判断是否为幺九牌（包括数牌幺九和字牌）
 * @param [in] tile 牌
 * @return bool
 */
static forceinline bool is_terminal_or_honor(tile_t tile) {
    return is_terminal(tile) || is_honor(tile);
}

/**
 * @brief 判断两张牌花色是否相同（更快）
 *  函数不检查输入的合法性。如果输入不合法的值，将无法保证合法返回值的合法性
 * @param [in] tile0 牌0
 * @param [in] tile1 牌1
 * @return bool
 */
static forceinline bool is_suit_equal_quick(tile_t tile0, tile_t tile1) {
    // 算法原理：高4bit表示花色
    return ((tile0 & 0xF0) == (tile1 & 0xF0));
}

/**
 * @brief 判断两张牌点数是否相同（更快）
 *  函数不检查输入的合法性。如果输入不合法的值，将无法保证合法返回值的合法性
 * @param [in] tile0 牌0
 * @param [in] tile1 牌1
 * @return bool
 */
static forceinline bool is_rank_equal_quick(tile_t tile0, tile_t tile1) {
    // 算法原理：低4bit表示花色。高4bit设置为C是为了过滤掉字牌
    return ((tile0 & 0xCF) == (tile1 & 0xCF));
}

/**
 * end group
 * @}
 */

}

namespace mahjong {

/**
 * @brief 字符串格式：
 * - 数牌：万=m 条=s 饼=p。后缀使用小写字母，一连串同花色的数牌可合并使用用一个后缀，如123m、678s等等。
 * - 字牌：东南西北=ESWN，中发白=CFP。使用大写字母。亦兼容天凤风格的后缀z，但按中国习惯顺序567z为中发白。
 * - 吃、碰、杠用英文[]，可选用逗号+数字表示供牌来源。数字的具体规则如下：
 *    - 吃：表示第几张牌是由上家打出，如[567m,2]表示57万吃6万（第2张）。对于不指定数字的，默认为吃第1张。
 *    - 碰：表示由哪家打出，1为上家，2为对家，3为下家，如[999s,3]表示碰下家的9条。对于不指定数字的，默认为碰上家。
 *    - 杠：与碰类似，但对于不指定数字的，则认为是暗杠。例如：[SSSS]表示暗杠南；[8888p,1]表示明杠上家的8饼。
 * - 范例
 *    - [EEEE][CCCC][FFFF][PPPP]NN
 *    - 1112345678999s9s
 *    - [WWWW,1][444s]45m678pFF6m
 *    - [EEEE]288s349pSCFF2p
 *    - [123p,1][345s,2][999s,3]6m6pEW1m
 *    - 356m18s1579pWNFF9p
 */

/**
 * @addtogroup stringify
 * @{
 */

/**
 * @name error codes
 * @{
 *  解析牌的错误码
 */
#define PARSE_NO_ERROR 0                                ///< 无错误
#define PARSE_ERROR_ILLEGAL_CHARACTER -1                ///< 非法字符
#define PARSE_ERROR_NO_SUFFIX_AFTER_DIGIT -2            ///< 数字后面缺少后缀
#define PARSE_ERROR_WRONG_TILES_COUNT_FOR_FIXED_PACK -3 ///< 副露包含错误的牌数目
#define PARSE_ERROR_CANNOT_MAKE_FIXED_PACK -4           ///< 无法正确解析副露
#define PARSE_ERROR_TOO_MANY_FIXED_PACKS -5             ///< 过多组副露（一副合法手牌最多4副露）
/**
 * @}
 */

/**
 * @brief 解析牌
 * @param [in] str 字符串
 * @param [out] tiles 牌
 * @param [in] max_cnt 牌的最大数量
 * @retval > 0 实际牌的数量
 * @retval == 0 失败
 */
intptr_t parse_tiles(const char *str, tile_t *tiles, intptr_t max_cnt);

/**
 * @brief 字符串转换为手牌结构和上牌
 * @param [in] str 字符串
 * @param [out] hand_tiles 手牌结构
 * @param [out] serving_tile 上的牌
 * @retval PARSE_NO_ERROR 无错误
 * @retval PARSE_ERROR_ILLEGAL_CHARACTER 非法字符
 * @retval PARSE_ERROR_NO_SUFFIX_AFTER_DIGIT 数字后面缺少后缀
 * @retval PARSE_ERROR_WRONG_TILES_COUNT_FOR_FIXED_PACK 副露包含错误的牌数目
 * @retval PARSE_ERROR_CANNOT_MAKE_FIXED_PACK 无法正确解析副露
 * @retval PARSE_ERROR_TOO_MANY_FIXED_PACKS 过多组副露（一副合法手牌最多4副露）
 */
intptr_t string_to_tiles(const char *str, hand_tiles_t *hand_tiles, tile_t *serving_tile);

/**
 * @brief 牌转换为字符串
 * @param [in] tiles 牌
 * @param [in] tile_cnt 牌的数量
 * @param [out] str 字符串
 * @param [in] max_size 字符串最大长度
 * @return intptr_t 写入的字符串数
 */
intptr_t tiles_to_string(const tile_t *tiles, intptr_t tile_cnt, char *str, intptr_t max_size);

/**
 * @brief 牌组转换为字符串
 * @param [in] packs 牌组
 * @param [in] pack_cnt 牌组的数量
 * @param [out] str 字符串
 * @param [in] max_size 字符串最大长度
 * @return intptr_t 写入的字符串数
 */
intptr_t packs_to_string(const pack_t *packs, intptr_t pack_cnt, char *str, intptr_t max_size);

/**
 * @brief 手牌结构转换为字符串
 * @param [in] hand_tiles 手牌结构
 * @param [out] str 字符串
 * @param [in] max_size 字符串最大长度
 * @return intptr_t 写入的字符串数
 */
intptr_t hand_tiles_to_string(const hand_tiles_t *hand_tiles, char *str, intptr_t max_size);

/**
 * end group
 * @}
 */

}

namespace mahjong {

// 十三幺13面听
static const tile_t standard_thirteen_orphans[13] = {
    TILE_1m, TILE_9m, TILE_1s, TILE_9s, TILE_1p, TILE_9p, TILE_E, TILE_S, TILE_W, TILE_N, TILE_C, TILE_F, TILE_P
};

// 组合龙只有如下6种
// 147m 258s 369p
// 147m 369s 258p
// 258m 147s 369p
// 258m 369s 147p
// 369m 147s 258p
// 369m 258s 147p
static const tile_t standard_knitted_straight[6][9] = {
    { TILE_1m, TILE_4m, TILE_7m, TILE_2s, TILE_5s, TILE_8s, TILE_3p, TILE_6p, TILE_9p },
    { TILE_1m, TILE_4m, TILE_7m, TILE_3s, TILE_6s, TILE_9s, TILE_2p, TILE_5p, TILE_8p },
    { TILE_2m, TILE_5m, TILE_8m, TILE_1s, TILE_4s, TILE_7s, TILE_3p, TILE_6p, TILE_9p },
    { TILE_2m, TILE_5m, TILE_8m, TILE_3s, TILE_6s, TILE_9s, TILE_1p, TILE_4p, TILE_7p },
    { TILE_3m, TILE_6m, TILE_9m, TILE_1s, TILE_4s, TILE_7s, TILE_2p, TILE_5p, TILE_8p },
    { TILE_3m, TILE_6m, TILE_9m, TILE_2s, TILE_5s, TILE_8s, TILE_1p, TILE_4p, TILE_7p },
};

}

namespace mahjong {

/**
 * @brief 牌组转换成牌
 *
 * @param [in] packs 牌组
 * @param [in] pack_cnt 牌组的数量
 * @param [out] tiles 牌
 * @param [in] tile_cnt 牌的最大数量
 * @return intptr_t 牌的实际数量
 */
intptr_t packs_to_tiles(const pack_t *packs, intptr_t pack_cnt, tile_t *tiles, intptr_t tile_cnt);

/**
 * @brief 将牌打表
 *
 * @param [in] tiles 牌
 * @param [in] cnt 牌的数量
 * @param [out] cnt_table 牌的数量表
 */
void map_tiles(const tile_t *tiles, intptr_t cnt, tile_table_t *cnt_table);

/**
 * @brief 将手牌打表
 *
 * @param [in] hand_tiles 手牌
 * @param [out] cnt_table 牌的数量表
 * @return bool 手牌结构是否正确。即是否符合：副露组数*3+立牌数=13
 */
bool map_hand_tiles(const hand_tiles_t *hand_tiles, tile_table_t *cnt_table);

/**
 * @brief 将表转换成牌
 *
 * @param [in] cnt_table 牌的数量表
 * @param [out] tiles 牌
 * @param [in] max_cnt 牌的最大数量
 * @return intptr_t 牌的实际数量
 */
intptr_t table_to_tiles(const tile_table_t &cnt_table, tile_t *tiles, intptr_t max_cnt);

/**
 * @brief 有效牌标记表类型
 */
typedef bool useful_table_t[TILE_TABLE_SIZE];

/**
 * @brief 计数有效牌枚数
 *
 * @param [in] used_table 已经的使用牌的数量表
 * @param [in] useful_table 有效牌标记表
 * @return int 有效牌枚数
 */
int count_useful_tile(const tile_table_t &used_table, const useful_table_t &useful_table);

/**
 * @addtogroup shanten
 * @{
 */

/**
 * @addtogroup basic_form
 * @{
 */

/**
 * @brief 基本和型上听数
 *
 * @param [in] standing_tiles 立牌
 * @param [in] standing_cnt 立牌数
 * @param [out] useful_table 有效牌标记表（可为null）
 * @return int 上听数
 */
int basic_form_shanten(const tile_t *standing_tiles, intptr_t standing_cnt, useful_table_t *useful_table);

/**
 * @brief 基本和型是否听牌
 *
 * @param [in] standing_tiles 立牌
 * @param [in] standing_cnt 立牌数
 * @param [out] waiting_table 听牌标记表（可为null）
 * @return bool 是否听牌
 */
bool is_basic_form_wait(const tile_t *standing_tiles, intptr_t standing_cnt, useful_table_t *waiting_table);

/**
 * @brief 基本和型是否和牌
 *
 * @param [in] standing_tiles 立牌
 * @param [in] standing_cnt 立牌数
 * @param [in] test_tile 测试的牌
 * @return bool 是否和牌
 */
bool is_basic_form_win(const tile_t *standing_tiles, intptr_t standing_cnt, tile_t test_tile);

/**
 * end group
 * @}
 */

/**
 * @addtogroup seven_pairs
 * @{
 */

/**
 * @brief 七对上听数
 *
 * @param [in] standing_tiles 立牌
 * @param [in] standing_cnt 立牌数
 * @param [out] useful_table 有效牌标记表（可为null）
 * @return int 上听数
 */
int seven_pairs_shanten(const tile_t *standing_tiles, intptr_t standing_cnt, useful_table_t *useful_table);

/**
 * @brief 七对是否听牌
 *
 * @param [in] standing_tiles 立牌
 * @param [in] standing_cnt 立牌数
 * @param [out] waiting_table 听牌标记表（可为null）
 * @return bool 是否听牌
 */
bool is_seven_pairs_wait(const tile_t *standing_tiles, intptr_t standing_cnt, useful_table_t *waiting_table);

/**
 * @brief 七对是否和牌
 *
 * @param [in] standing_tiles 立牌
 * @param [in] standing_cnt 立牌数
 * @param [in] test_tile 测试的牌
 * @return bool 是否和牌
 */
bool is_seven_pairs_win(const tile_t *standing_tiles, intptr_t standing_cnt, tile_t test_tile);

/**
 * end group
 * @}
 */

/**
 * @addtogroup thirteen_orphans
 * @{
 */

/**
 * @brief 十三幺上听数
 *
 * @param [in] standing_tiles 立牌
 * @param [in] standing_cnt 立牌数
 * @param [out] useful_table 有效牌标记表（可为null）
 * @return int 上听数
 */
int thirteen_orphans_shanten(const tile_t *standing_tiles, intptr_t standing_cnt, useful_table_t *useful_table);

/**
 * @brief 十三幺是否听牌
 *
 * @param [in] standing_tiles 立牌
 * @param [in] standing_cnt 立牌数
 * @param [out] waiting_table 听牌标记表（可为null）
 * @return bool 是否听牌
 */
bool is_thirteen_orphans_wait(const tile_t *standing_tiles, intptr_t standing_cnt, useful_table_t *waiting_table);

/**
 * @brief 十三幺是否和牌
 *
 * @param [in] standing_tiles 立牌
 * @param [in] standing_cnt 立牌数
 * @param [in] test_tile 测试的牌
 * @return bool 是否和牌
 */
bool is_thirteen_orphans_win(const tile_t *standing_tiles, intptr_t standing_cnt, tile_t test_tile);

/**
 * end group
 * @}
 */

/**
 * @addtogroup knitted_straight
 * @{
 */

/**
 * @brief 组合龙上听数
 *
 * @param [in] standing_tiles 立牌
 * @param [in] standing_cnt 立牌数
 * @param [out] useful_table 有效牌标记表（可为null）
 * @return int 上听数
 */
int knitted_straight_shanten(const tile_t *standing_tiles, intptr_t standing_cnt, useful_table_t *useful_table);

/**
 * @brief 组合龙是否听牌
 *
 * @param [in] standing_tiles 立牌
 * @param [in] standing_cnt 立牌数
 * @param [out] waiting_table 听牌标记表（可为null）
 * @return bool 是否听牌
 */
bool is_knitted_straight_wait(const tile_t *standing_tiles, intptr_t standing_cnt, useful_table_t *waiting_table);

/**
 * @brief 组合龙是否和牌
 *
 * @param [in] standing_tiles 立牌
 * @param [in] standing_cnt 立牌数
 * @param [in] test_tile 测试的牌
 * @return bool 是否和牌
 */
bool is_knitted_straight_win(const tile_t *standing_tiles, intptr_t standing_cnt, tile_t test_tile);

/**
 * end group
 * @}
 */

/**
 * @addtogroup honors_and_knitted_tiles
 * @{
 */

/**
 * @brief 全不靠上听数
 *
 * @param [in] standing_tiles 立牌
 * @param [in] standing_cnt 立牌数
 * @param [out] useful_table 有效牌标记表（可为null）
 * @return int 上听数
 */
int honors_and_knitted_tiles_shanten(const tile_t *standing_tiles, intptr_t standing_cnt, useful_table_t *useful_table);

/**
 * @brief 全不靠是否听牌
 *
 * @param [in] standing_tiles 立牌
 * @param [in] standing_cnt 立牌数
 * @param [out] waiting_table 听牌标记表（可为null）
 * @return bool 是否听牌
 */
bool is_honors_and_knitted_tiles_wait(const tile_t *standing_tiles, intptr_t standing_cnt, useful_table_t *waiting_table);

/**
 * @brief 全不靠是否和牌
 *
 * @param [in] standing_tiles 立牌
 * @param [in] standing_cnt 立牌数
 * @param [in] test_tile 测试的牌
 * @return bool 是否和牌
 */
bool is_honors_and_knitted_tiles_win(const tile_t *standing_tiles, intptr_t standing_cnt, tile_t test_tile);

/**
 * end group
 * @}
 */

/**
 * end group
 * @}
 */

/**
 * @name form flags
 * @{
 *  和型
 */
#define FORM_FLAG_BASIC_FORM                0x01  ///< 基本和型
#define FORM_FLAG_SEVEN_PAIRS               0x02  ///< 七对
#define FORM_FLAG_THIRTEEN_ORPHANS          0x04  ///< 十三幺
#define FORM_FLAG_HONORS_AND_KNITTED_TILES  0x08  ///< 全不靠
#define FORM_FLAG_KNITTED_STRAIGHT          0x10  ///< 组合龙
#define FORM_FLAG_ALL                       0xFF  ///< 全部和型
/**
 * @}
 */

/**
 * @brief 枚举打哪张牌的计算结果信息
 */
struct enum_result_t {
    tile_t discard_tile;                    ///< 打这张牌
    uint8_t form_flag;                      ///< 和牌形式
    int shanten;                            ///< 上听数
    useful_table_t useful_table;            ///< 有效牌标记表
};

/**
 * @brief 枚举打哪张牌的计算回调函数
 *
 * @param [in] context 从enum_discard_tile传过来的context原样传回
 * @param [in] result 计算结果
 * @retval true 继续枚举
 * @retval false 结束枚举
 */
typedef bool (*enum_callback_t)(void *context, const enum_result_t *result);

/**
 * @brief 枚举打哪张牌
 *
 * @param [in] hand_tiles 手牌结构
 * @param [in] serving_tile 上牌（可为0，此时仅计算手牌的信息）
 * @param [in] form_flag 计算哪些和型
 * @param [in] context 用户自定义参数，将原样从回调函数传回
 * @param [in] enum_callback 回调函数
 */
void enum_discard_tile(const hand_tiles_t *hand_tiles, tile_t serving_tile, uint8_t form_flag,
    void *context, enum_callback_t enum_callback);

}

#define SUPPORT_CONCEALED_KONG_AND_MELDED_KONG 1  // 支持明暗杠

namespace mahjong {

/**
 * @addtogroup calculator
 * @{
 */

/**
 * @brief 番种
 */
enum fan_t {
    FAN_NONE = 0,                       ///< 无效
    BIG_FOUR_WINDS,                     ///< 大四喜
    BIG_THREE_DRAGONS,                  ///< 大三元
    ALL_GREEN,                          ///< 绿一色
    NINE_GATES,                         ///< 九莲宝灯
    FOUR_KONGS,                         ///< 四杠
    SEVEN_SHIFTED_PAIRS,                ///< 连七对
    THIRTEEN_ORPHANS,                   ///< 十三幺

    ALL_TERMINALS,                      ///< 清幺九
    LITTLE_FOUR_WINDS,                  ///< 小四喜
    LITTLE_THREE_DRAGONS,               ///< 小三元
    ALL_HONORS,                         ///< 字一色
    FOUR_CONCEALED_PUNGS,               ///< 四暗刻
    PURE_TERMINAL_CHOWS,                ///< 一色双龙会

    QUADRUPLE_CHOW,                     ///< 一色四同顺
    FOUR_PURE_SHIFTED_PUNGS,            ///< 一色四节高

    FOUR_PURE_SHIFTED_CHOWS,            ///< 一色四步高
    THREE_KONGS,                        ///< 三杠
    ALL_TERMINALS_AND_HONORS,           ///< 混幺九

    SEVEN_PAIRS,                        ///< 七对
    GREATER_HONORS_AND_KNITTED_TILES,   ///< 七星不靠
    ALL_EVEN_PUNGS,                     ///< 全双刻
    FULL_FLUSH,                         ///< 清一色
    PURE_TRIPLE_CHOW,                   ///< 一色三同顺
    PURE_SHIFTED_PUNGS,                 ///< 一色三节高
    UPPER_TILES,                        ///< 全大
    MIDDLE_TILES,                       ///< 全中
    LOWER_TILES,                        ///< 全小

    PURE_STRAIGHT,                      ///< 清龙
    THREE_SUITED_TERMINAL_CHOWS,        ///< 三色双龙会
    PURE_SHIFTED_CHOWS,                 ///< 一色三步高
    ALL_FIVE,                           ///< 全带五
    TRIPLE_PUNG,                        ///< 三同刻
    THREE_CONCEALED_PUNGS,              ///< 三暗刻

    LESSER_HONORS_AND_KNITTED_TILES,    ///< 全不靠
    KNITTED_STRAIGHT,                   ///< 组合龙
    UPPER_FOUR,                         ///< 大于五
    LOWER_FOUR,                         ///< 小于五
    BIG_THREE_WINDS,                    ///< 三风刻

    MIXED_STRAIGHT,                     ///< 花龙
    REVERSIBLE_TILES,                   ///< 推不倒
    MIXED_TRIPLE_CHOW,                  ///< 三色三同顺
    MIXED_SHIFTED_PUNGS,                ///< 三色三节高
    CHICKEN_HAND,                       ///< 无番和
    LAST_TILE_DRAW,                     ///< 妙手回春
    LAST_TILE_CLAIM,                    ///< 海底捞月
    OUT_WITH_REPLACEMENT_TILE,          ///< 杠上开花
    ROBBING_THE_KONG,                   ///< 抢杠和

    ALL_PUNGS,                          ///< 碰碰和
    HALF_FLUSH,                         ///< 混一色
    MIXED_SHIFTED_CHOWS,                ///< 三色三步高
    ALL_TYPES,                          ///< 五门齐
    MELDED_HAND,                        ///< 全求人
    TWO_CONCEALED_KONGS,                ///< 双暗杠
    TWO_DRAGONS_PUNGS,                  ///< 双箭刻

    OUTSIDE_HAND,                       ///< 全带幺
    FULLY_CONCEALED_HAND,               ///< 不求人
    TWO_MELDED_KONGS,                   ///< 双明杠
    LAST_TILE,                          ///< 和牌张

    DRAGON_PUNG,                        ///< 箭刻
    PREVALENT_WIND,                     ///< 圈风刻
    SEAT_WIND,                          ///< 门风刻
    CONCEALED_HAND,                     ///< 门前清
    ALL_CHOWS,                          ///< 平和
    TILE_HOG,                           ///< 四归一
    DOUBLE_PUNG,                        ///< 双同刻
    TWO_CONCEALED_PUNGS,                ///< 双暗刻
    CONCEALED_KONG,                     ///< 暗杠
    ALL_SIMPLES,                        ///< 断幺

    PURE_DOUBLE_CHOW,                   ///< 一般高
    MIXED_DOUBLE_CHOW,                  ///< 喜相逢
    SHORT_STRAIGHT,                     ///< 连六
    TWO_TERMINAL_CHOWS,                 ///< 老少副
    PUNG_OF_TERMINALS_OR_HONORS,        ///< 幺九刻
    MELDED_KONG,                        ///< 明杠
    ONE_VOIDED_SUIT,                    ///< 缺一门
    NO_HONORS,                          ///< 无字
    EDGE_WAIT,                          ///< 边张
    CLOSED_WAIT,                        ///< 嵌张
    SINGLE_WAIT,                        ///< 单钓将
    SELF_DRAWN,                         ///< 自摸

    FLOWER_TILES,                       ///< 花牌

#if SUPPORT_CONCEALED_KONG_AND_MELDED_KONG
    CONCEALED_KONG_AND_MELDED_KONG,     ///< 明暗杠
#endif

    FAN_TABLE_SIZE
};

/**
 * @brief 风（用来表示圈风门风）
 */
enum class wind_t {
    EAST, SOUTH, WEST, NORTH
};

/**
 * @brief 和牌标记
 */
typedef uint8_t win_flag_t;

/**
 * @name win flag
 * @{
 */
#define WIN_FLAG_DISCARD    0   ///< 点和
#define WIN_FLAG_SELF_DRAWN 1   ///< 自摸
#define WIN_FLAG_4TH_TILE   2   ///< 绝张
#define WIN_FLAG_ABOUT_KONG 4   ///< 关于杠，复合点和时为枪杠和，复合自摸则为杠上开花
#define WIN_FLAG_WALL_LAST  8   ///< 牌墙最后一张，复合点和时为海底捞月，复合自摸则为妙手回春
/**
 * @}
 */

/**
 * @name error codes
 * @{
 */
#define ERROR_WRONG_TILES_COUNT -1              ///< 错误的张数
#define ERROR_TILE_COUNT_GREATER_THAN_4 -2      ///< 某张牌出现超过4枚
#define ERROR_NOT_WIN -3                        ///< 没和牌
/**
 * @}
 */

/**
 * @brief 检查算番的输入是否合法
 *
 * @param [in] hand_tiles 手牌
 * @param [in] win_tile 和牌张
 * @retval 0 成功
 * @retval ERROR_WRONG_TILES_COUNT 错误的张数
 * @retval ERROR_TILE_COUNT_GREATER_THAN_4 某张牌出现超过4枚
 */
int check_calculator_input(const hand_tiles_t *hand_tiles, tile_t win_tile);

/**
 * @brief 算番参数
 */
struct calculate_param_t {
    hand_tiles_t hand_tiles;    ///< 手牌
    tile_t win_tile;            ///< 和牌张
    uint8_t flower_count;       ///< 花牌数
    win_flag_t win_flag;        ///< 和牌标记
    wind_t prevalent_wind;      ///< 圈风
    wind_t seat_wind;           ///< 门风
};

/**
 * @brief 番表
 */
typedef uint16_t fan_table_t[FAN_TABLE_SIZE];

/**
 * @brief 算番
 *
 * @param [in] calculate_param 算番参数
 * @param [out] fan_table 番表，当有某种番时，相应的会设置为这种番出现的次数
 * @retval >0 番数
 * @retval ERROR_WRONG_TILES_COUNT 错误的张数
 * @retval ERROR_TILE_COUNT_GREATER_THAN_4 某张牌出现超过4枚
 * @retval ERROR_NOT_WIN 没和牌
 */
int calculate_fan(const calculate_param_t *calculate_param, fan_table_t *fan_table);

#if 0

/**
 * @brief 番名（英文）
 */
static const char *fan_name[] = {
    "None",
    "Big Four Winds", "Big Three Dragons", "All Green", "Nine Gates", "Four Kongs", "Seven Shifted Pairs", "Thirteen Orphans",
    "All Terminals", "Little Four Winds", "Little Three Dragons", "All Honors", "Four Concealed Pungs", "Pure Terminal Chows",
    "Quadruple Chow", "Four Pure Shifted Pungs",
    "Four Pure Shifted Chows", "Three Kongs", "All Terminals and Honors",
    "Seven Pairs", "Greater Honors and Knitted Tiles", "All Even Pungs", "Full Flush", "Pure Triple Chow", "Pure Shifted Pungs", "Upper Tiles", "Middle Tiles", "Lower Tiles",
    "Pure Straight", "Three-Suited Terminal Chows", "Pure Shifted Chows", "All Five", "Triple Pung", "Three Concealed Pungs",
    "Lesser Honors and Knitted Tiles", "Knitted Straight", "Upper Four", "Lower Four", "Big Three Winds",
    "Mixed Straight", "Reversible Tiles", "Mixed Triple Chow", "Mixed Shifted Pungs", "Chicken Hand", "Last Tile Draw", "Last Tile Claim", "Out with Replacement Tile", "Robbing The Kong",
    "All Pungs", "Half Flush", "Mixed Shifted Chows", "All Types", "Melded Hand", "Two Concealed Kongs", "Two Dragons Pungs",
    "Outside Hand", "Fully Concealed Hand", "Two Melded Kongs", "Last Tile",
    "Dragon Pung", "Prevalent Wind", "Seat Wind", "Concealed Hand", "All Chows", "Tile Hog", "Double Pung",
    "Two Concealed Pungs", "Concealed Kong", "All Simples",
    "Pure Double Chow", "Mixed Double Chow", "Short Straight", "Two Terminal Chows", "Pung of Terminals or Honors", "Melded Kong", "One Voided Suit", "No Honors", "Edge Wait", "Closed Wait", "Single Wait", "Self-Drawn",
    "Flower Tiles"
#if SUPPORT_CONCEALED_KONG_AND_MELDED_KONG
    , "Concealed Kong and Melded Kong"
#endif
};

#else

#ifdef _MSC_VER
#pragma execution_character_set("utf-8")
#endif

#ifndef __UTF8_TEXT
// VS2015 GCC4.7 Clang5.0
#if (defined(_MSC_VER) && (_MSC_VER >= 1900)) || (defined(__GNUC__) && ((__GNUC__ << 8 | __GNUC_MINOR__) >= 0x407)) || (defined(__clang__) && (__clang_major__ >= 5))
#define __UTF8_TEXT(quote) u8 ## quote
#else
#define __UTF8_TEXT(quote) quote
#endif
#endif
    
#ifndef __UTF8
#define __UTF8(quote) __UTF8_TEXT(quote)
#endif

/**
 * @brief 番名（简体中文）
 */
extern const char *fan_name[];

#endif

/**
 * @brief 番值
 */
static const uint16_t fan_value_table[FAN_TABLE_SIZE] = {
    0,
    88, 88, 88, 88, 88, 88, 88,
    64, 64, 64, 64, 64, 64,
    48, 48,
    32, 32, 32,
    24, 24, 24, 24, 24, 24, 24, 24, 24,
    16, 16, 16, 16, 16, 16,
    12, 12, 12, 12, 12,
    8, 8, 8, 8, 8, 8, 8, 8, 8,
    6, 6, 6, 6, 6, 6, 6,
    4, 4, 4, 4,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1
#if SUPPORT_CONCEALED_KONG_AND_MELDED_KONG
    , 5
#endif
};

/**
 * @brief 判断立牌是否包含和牌
 * 如果是，则必然不是和绝张
 *
 * @param [in] standing_tiles 立牌
 * @param [in] standing_cnt 立牌数
 * @param [in] win_tile 和牌张
 * @return bool
 */
bool is_standing_tiles_contains_win_tile(const tile_t *standing_tiles, intptr_t standing_cnt, tile_t win_tile);

/**
 * @brief 统计和牌在副露牌组中出现的张数
 * 如果出现3张，则必然和绝张
 *
 * @param [in] fixed_packs 副露牌组
 * @param [in] fixed_cnt 副露牌组数
 * @param [in] win_tile 和牌张
 * @return size_t
 */
size_t count_win_tile_in_fixed_packs(const pack_t *fixed_packs, intptr_t fixed_cnt, tile_t win_tile);

/**
 * @brief 判断副露牌组是否包含杠
 *
 * @param [in] fixed_packs 副露牌组
 * @param [in] fixed_cnt 副露牌组数
 * @return bool
 */
bool is_fixed_packs_contains_kong(const pack_t *fixed_packs, intptr_t fixed_cnt);

/**
 * end group
 * @}
 */

}

using namespace std;

void MahjongInit();

vector<pair<int, string> > MahjongFanCalculator(
    vector<pair<string, pair<string, int> > > pack,
    vector<string> hand,
    string winTile,
    int flowerCount,
    bool isZIMO,
    bool isJUEZHANG,
    bool isGANG,
    bool isLAST,
    int menFeng,
    int quanFeng);

#endif
