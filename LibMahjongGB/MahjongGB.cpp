﻿#include "MahjongGB.h"

namespace mahjong {

/**
 * @brief 番名（简体中文）
 */
const char *fan_name[] = {
    __UTF8("无"),
    __UTF8("大四喜"), __UTF8("大三元"), __UTF8("绿一色"), __UTF8("九莲宝灯"), __UTF8("四杠"), __UTF8("连七对"), __UTF8("十三幺"),
    __UTF8("清幺九"), __UTF8("小四喜"), __UTF8("小三元"), __UTF8("字一色"), __UTF8("四暗刻"), __UTF8("一色双龙会"),
    __UTF8("一色四同顺"), __UTF8("一色四节高"),
    __UTF8("一色四步高"), __UTF8("三杠"), __UTF8("混幺九"),
    __UTF8("七对"), __UTF8("七星不靠"), __UTF8("全双刻"), __UTF8("清一色"), __UTF8("一色三同顺"), __UTF8("一色三节高"), __UTF8("全大"), __UTF8("全中"), __UTF8("全小"),
    __UTF8("清龙"), __UTF8("三色双龙会"), __UTF8("一色三步高"), __UTF8("全带五"), __UTF8("三同刻"), __UTF8("三暗刻"),
    __UTF8("全不靠"), __UTF8("组合龙"), __UTF8("大于五"), __UTF8("小于五"), __UTF8("三风刻"),
    __UTF8("花龙"), __UTF8("推不倒"), __UTF8("三色三同顺"), __UTF8("三色三节高"), __UTF8("无番和"), __UTF8("妙手回春"), __UTF8("海底捞月"), __UTF8("杠上开花"), __UTF8("抢杠和"),
    __UTF8("碰碰和"), __UTF8("混一色"), __UTF8("三色三步高"), __UTF8("五门齐"), __UTF8("全求人"), __UTF8("双暗杠"), __UTF8("双箭刻"),
    __UTF8("全带幺"), __UTF8("不求人"), __UTF8("双明杠"), __UTF8("和绝张"),
    __UTF8("箭刻"), __UTF8("圈风刻"), __UTF8("门风刻"), __UTF8("门前清"), __UTF8("平和"), __UTF8("四归一"), __UTF8("双同刻"), __UTF8("双暗刻"), __UTF8("暗杠"), __UTF8("断幺"),
    __UTF8("一般高"), __UTF8("喜相逢"), __UTF8("连六"), __UTF8("老少副"), __UTF8("幺九刻"), __UTF8("明杠"), __UTF8("缺一门"), __UTF8("无字"), __UTF8("边张"), __UTF8("嵌张"), __UTF8("单钓将"), __UTF8("自摸"),
    __UTF8("花牌")
#if SUPPORT_CONCEALED_KONG_AND_MELDED_KONG
    , __UTF8("明暗杠")
#endif
};

// 解析牌实现函数
static intptr_t parse_tiles_impl(const char *str, tile_t *tiles, intptr_t max_cnt, intptr_t *out_tile_cnt) {
    //if (strspn(str, "123456789mpsESWNCFP") != strlen(str)) {
    //    return PARSE_ERROR_ILLEGAL_CHARACTER;
    //}

    intptr_t tile_cnt = 0;

#define SET_SUIT_FOR_NUMBERED(value_)       \
    for (intptr_t i = tile_cnt; i > 0;) {   \
        if (tiles[--i] & 0xF0) break;       \
        tiles[i] |= value_;                 \
        } (void)0

#define SET_SUIT_FOR_HONOR() \
    for (intptr_t i = tile_cnt; i > 0;) {   \
        if (tiles[--i] & 0xF0) break;       \
        if (tiles[i] > 7) return PARSE_ERROR_ILLEGAL_CHARACTER; \
        tiles[i] |= 0x40;                   \
        } (void)0

#define NO_SUFFIX_AFTER_DIGIT() (tile_cnt > 0 && !(tiles[tile_cnt - 1] & 0xF0))
#define CHECK_SUFFIX() if (NO_SUFFIX_AFTER_DIGIT()) return PARSE_ERROR_NO_SUFFIX_AFTER_DIGIT

    const char *p = str;
    for (; tile_cnt < max_cnt && *p != '\0'; ++p) {
        char c = *p;
        switch (c) {
        case '0': tiles[tile_cnt++] = 5; break;
        case '1': tiles[tile_cnt++] = 1; break;
        case '2': tiles[tile_cnt++] = 2; break;
        case '3': tiles[tile_cnt++] = 3; break;
        case '4': tiles[tile_cnt++] = 4; break;
        case '5': tiles[tile_cnt++] = 5; break;
        case '6': tiles[tile_cnt++] = 6; break;
        case '7': tiles[tile_cnt++] = 7; break;
        case '8': tiles[tile_cnt++] = 8; break;
        case '9': tiles[tile_cnt++] = 9; break;
        case 'm': SET_SUIT_FOR_NUMBERED(0x10); break;
        case 's': SET_SUIT_FOR_NUMBERED(0x20); break;
        case 'p': SET_SUIT_FOR_NUMBERED(0x30); break;
        case 'z': SET_SUIT_FOR_HONOR(); break;
        case 'E': CHECK_SUFFIX(); tiles[tile_cnt++] = TILE_E; break;
        case 'S': CHECK_SUFFIX(); tiles[tile_cnt++] = TILE_S; break;
        case 'W': CHECK_SUFFIX(); tiles[tile_cnt++] = TILE_W; break;
        case 'N': CHECK_SUFFIX(); tiles[tile_cnt++] = TILE_N; break;
        case 'C': CHECK_SUFFIX(); tiles[tile_cnt++] = TILE_C; break;
        case 'F': CHECK_SUFFIX(); tiles[tile_cnt++] = TILE_F; break;
        case 'P': CHECK_SUFFIX(); tiles[tile_cnt++] = TILE_P; break;
        default: goto parse_finish;
        }
    }

parse_finish:
    // 一连串数字+后缀，但已经超过容量，那么放弃中间一部分数字，直接解析最近的后缀
    if (NO_SUFFIX_AFTER_DIGIT()) {
        const char *p1 = strpbrk(p, "mspz");
        if (p1 == nullptr) {
            return PARSE_ERROR_NO_SUFFIX_AFTER_DIGIT;
        }

        switch (*p1) {
        case 'm': SET_SUIT_FOR_NUMBERED(0x10); break;
        case 's': SET_SUIT_FOR_NUMBERED(0x20); break;
        case 'p': SET_SUIT_FOR_NUMBERED(0x30); break;
        case 'z': SET_SUIT_FOR_HONOR(); break;
        default: return PARSE_ERROR_NO_SUFFIX_AFTER_DIGIT;
        }
        p = p1 + 1;
    }

#undef SET_SUIT_FOR_NUMBERED
#undef SET_SUIT_FOR_HONOR
#undef NO_SUFFIX_AFTER_DIGIT
#undef CHECK_SUFFIX

    if (out_tile_cnt != nullptr) {
        *out_tile_cnt = tile_cnt;
    }
    return static_cast<intptr_t>(p - str);
}

// 解析牌
intptr_t parse_tiles(const char *str, tile_t *tiles, intptr_t max_cnt) {
    intptr_t tile_cnt;
    if (parse_tiles_impl(str, tiles, max_cnt, &tile_cnt) > 0) {
        return tile_cnt;
    }
    return 0;
}

// 生成副露
static intptr_t make_fixed_pack(const tile_t *tiles, intptr_t tile_cnt, pack_t *pack, uint8_t offer) {
    if (tile_cnt > 0) {
        if (tile_cnt != 3 && tile_cnt != 4) {
            return PARSE_ERROR_WRONG_TILES_COUNT_FOR_FIXED_PACK;
        }
        if (tile_cnt == 3) {
            if (offer == 0) {
                offer = 1;
            }
            if (tiles[0] == tiles[1] && tiles[1] == tiles[2]) {
                *pack = make_pack(offer, PACK_TYPE_PUNG, tiles[0]);
            }
            else {
                if (tiles[0] + 1 == tiles[1] && tiles[1] + 1 == tiles[2]) {
                    *pack = make_pack(offer, PACK_TYPE_CHOW, tiles[1]);
                }
                else if (tiles[0] + 1 == tiles[2] && tiles[2] + 1 == tiles[1]) {
                    *pack = make_pack(offer, PACK_TYPE_CHOW, tiles[2]);
                }
                else if (tiles[1] + 1 == tiles[0] && tiles[0] + 1 == tiles[2]) {
                    *pack = make_pack(offer, PACK_TYPE_CHOW, tiles[0]);
                }
                else if (tiles[1] + 1 == tiles[2] && tiles[2] + 1 == tiles[0]) {
                    *pack = make_pack(offer, PACK_TYPE_CHOW, tiles[2]);
                }
                else if (tiles[2] + 1 == tiles[0] && tiles[0] + 1 == tiles[1]) {
                    *pack = make_pack(offer, PACK_TYPE_CHOW, tiles[0]);
                }
                else if (tiles[2] + 1 == tiles[1] && tiles[1] + 1 == tiles[0]) {
                    *pack = make_pack(offer, PACK_TYPE_CHOW, tiles[1]);
                }
                else {
                    return PARSE_ERROR_CANNOT_MAKE_FIXED_PACK;
                }
            }
        }
        else {
            if (tiles[0] != tiles[1] || tiles[1] != tiles[2] || tiles[2] != tiles[3]) {
                return PARSE_ERROR_CANNOT_MAKE_FIXED_PACK;
            }
            *pack = make_pack(offer, PACK_TYPE_KONG, tiles[0]);
        }
        return 1;
    }
    return 0;
}

// 字符串转换为手牌结构和上牌
intptr_t string_to_tiles(const char *str, hand_tiles_t *hand_tiles, tile_t *serving_tile) {
    size_t len = strlen(str);
    if (strspn(str, "0123456789mpszESWNCFP,[]") != len) {
        return PARSE_ERROR_ILLEGAL_CHARACTER;
    }

    pack_t packs[4];
    intptr_t pack_cnt = 0;
    bool in_brackets = false;
    tile_t tiles[14];
    intptr_t tile_cnt = 0;
    uint8_t offer = 0;

    const char *p = str;
    while (char c = *p) {
        const char *q;
        switch (c) {
        case ',': {
            if (!in_brackets) {
                return PARSE_ERROR_ILLEGAL_CHARACTER;
            }
            offer = static_cast<uint8_t>(*++p - '0');
            q = ++p;
            break;
        }
        case '[': {
            if (pack_cnt > 4) {
                return PARSE_ERROR_TOO_MANY_FIXED_PACKS;
            }
            q = ++p;
            in_brackets = true;
            offer = 0;
            break;
        }
        case ']': {
            if (!in_brackets) {
                return PARSE_ERROR_ILLEGAL_CHARACTER;
            }
            q = ++p;
            intptr_t ret = make_fixed_pack(tiles, tile_cnt, &packs[pack_cnt], offer);
            if (ret < 0) {
                return ret;
            }
            in_brackets = false;
            ++pack_cnt;
            tile_cnt = 0;
            break;
        }
        default: {
            intptr_t ret = parse_tiles_impl(p, tiles, 14, &tile_cnt);
            if (ret < 0) {
                return ret;
            }
            if (ret == 0) {
                return PARSE_ERROR_ILLEGAL_CHARACTER;
            }
            q = p + ret;
            break;
        }
        }
        p = q;
    }

    memcpy(hand_tiles->fixed_packs, packs, pack_cnt * sizeof(pack_t));
    hand_tiles->pack_count = pack_cnt;
    intptr_t max_cnt = 13 - pack_cnt * 3;
    if (tile_cnt > max_cnt) {
        memcpy(hand_tiles->standing_tiles, tiles, max_cnt * sizeof(tile_t));
        hand_tiles->tile_count = max_cnt;
        if (serving_tile != nullptr) {
            *serving_tile = tiles[max_cnt];
        }
    }
    else {
        memcpy(hand_tiles->standing_tiles, tiles, tile_cnt * sizeof(tile_t));
        hand_tiles->tile_count = tile_cnt;
        if (serving_tile != nullptr) {
            *serving_tile = 0;
        }
    }

    return PARSE_NO_ERROR;
}

// 牌转换为字符串
intptr_t tiles_to_string(const tile_t *tiles, intptr_t tile_cnt, char *str, intptr_t max_size) {
    bool tenhon = false;
    char *p = str, *end = str + max_size;

    static const char suffix[] = "mspz";
    static const char honor_text[] = "ESWNCFP";
    suit_t last_suit = 0;
    for (intptr_t i = 0; i < tile_cnt && p < end; ++i) {
        tile_t t = tiles[i];
        suit_t s = tile_get_suit(t);
        rank_t r = tile_get_rank(t);
        if (s == 1 || s == 2 || s == 3) {  // 数牌
            if (r >= 1 && r <= 9) {  // 有效范围1-9
                if (last_suit != s && last_suit != 0) {  // 花色变了，加后缀
                    if (last_suit != 4 || tenhon) {
                        *p++ = suffix[last_suit - 1];
                    }
                }
                if (p < end) {
                    *p++ = '0' + r;  // 写入一个数字字符
                }
                last_suit = s;  // 记录花色
            }
        }
        else if (s == 4) {  // 字牌
            if (r >= 1 && r <= 7) {  // 有效范围1-7
                if (last_suit != s && last_suit != 0) {  // 花色变了，加后缀
                    if (last_suit != 4) {
                        *p++ = suffix[last_suit - 1];
                    }
                }
                if (p < end) {
                    if (tenhon) {  // 天凤式后缀
                        *p++ = '0' + r;  // 写入一个数字字符
                    }
                    else {
                        *p++ = honor_text[r - 1];  // 直接写入字牌相应字母
                    }
                    last_suit = s;
                }
            }
        }
    }

    // 写入过且还有空间，补充后缀
    if (p != str && p < end && (last_suit != 4 || tenhon)) {
        *p++ = suffix[last_suit - 1];
    }

    if (p < end) {
        *p = '\0';
    }
    return static_cast<intptr_t>(p - str);
}

// 牌组转换为字符串
intptr_t packs_to_string(const pack_t *packs, intptr_t pack_cnt, char *str, intptr_t max_size) {
    char *p = str, *end = str + max_size;
    tile_t temp[4];
    for (intptr_t i = 0; i < pack_cnt && p < end; ++i) {
        pack_t pack = packs[i];
        uint8_t o = pack_get_offer(pack);
        tile_t t = pack_get_tile(pack);
        uint8_t pt = pack_get_type(pack);
        switch (pt) {
        case PACK_TYPE_CHOW:
            if (p >= end) break;
            *p++ = '[';
            temp[0] = static_cast<tile_t>(t - 1); temp[1] = t; temp[2] = static_cast<tile_t>(t + 1);
            p += tiles_to_string(temp, 3, p, static_cast<intptr_t>(end - p));
            if (p >= end) break;
            *p++ = ',';
            if (p >= end) break;
            *p++ = '0' + o;
            if (p >= end) break;
            *p++ = ']';
            break;
        case PACK_TYPE_PUNG:
            if (p >= end) break;
            *p++ = '[';
            temp[0] = t; temp[1] = t; temp[2] = t;
            p += tiles_to_string(temp, 3, p, static_cast<intptr_t>(end - p));
            if (p >= end) break;
            *p++ = ',';
            if (p >= end) break;
            *p++ = '0' + o;
            if (p >= end) break;
            *p++ = ']';
            break;
        case PACK_TYPE_KONG:
            if (p >= end) break;
            *p++ = '[';
            temp[0] = t; temp[1] = t; temp[2] = t; temp[3] = t;
            p += tiles_to_string(temp, 4, p, static_cast<intptr_t>(end - p));
            if (p >= end) break;
            *p++ = ',';
            if (p >= end) break;
            *p++ = '0' + o;
            if (p >= end) break;
            *p++ = ']';

            //if (!is_pack_melded(pack))
            break;
        case PACK_TYPE_PAIR:
            temp[0] = t; temp[1] = t;
            p += tiles_to_string(temp, 2, p, static_cast<intptr_t>(end - p));
            break;
        default: break;
        }
    }

    if (p < end) {
        *p = '\0';
    }
    return static_cast<intptr_t>(p - str);
}

// 手牌结构转换为字符串
intptr_t hand_tiles_to_string(const hand_tiles_t *hand_tiles, char *str, intptr_t max_size) {
    char *p = str, *end = str + max_size;
    p += packs_to_string(hand_tiles->fixed_packs, hand_tiles->pack_count, str, max_size);
    if (p < end) p += tiles_to_string(hand_tiles->standing_tiles, hand_tiles->tile_count, p, static_cast<intptr_t>(end - p));
    return static_cast<intptr_t>(p - str);
}

}

namespace mahjong {

// 牌组转换成牌
intptr_t packs_to_tiles(const pack_t *packs, intptr_t pack_cnt, tile_t *tiles, intptr_t tile_cnt) {
    if (packs == nullptr || tiles == nullptr) {
        return 0;
    }

    intptr_t cnt = 0;
    for (int i = 0; i < pack_cnt && cnt < tile_cnt; ++i) {
        tile_t tile = pack_get_tile(packs[i]);
        switch (pack_get_type(packs[i])) {
        case PACK_TYPE_CHOW:
            if (cnt < tile_cnt) tiles[cnt++] = static_cast<tile_t>(tile - 1);
            if (cnt < tile_cnt) tiles[cnt++] = tile;
            if (cnt < tile_cnt) tiles[cnt++] = static_cast<tile_t>(tile + 1);
            break;
        case PACK_TYPE_PUNG:
            if (cnt < tile_cnt) tiles[cnt++] = tile;
            if (cnt < tile_cnt) tiles[cnt++] = tile;
            if (cnt < tile_cnt) tiles[cnt++] = tile;
            break;
        case PACK_TYPE_KONG:
            if (cnt < tile_cnt) tiles[cnt++] = tile;
            if (cnt < tile_cnt) tiles[cnt++] = tile;
            if (cnt < tile_cnt) tiles[cnt++] = tile;
            if (cnt < tile_cnt) tiles[cnt++] = tile;
            break;
        case PACK_TYPE_PAIR:
            if (cnt < tile_cnt) tiles[cnt++] = tile;
            if (cnt < tile_cnt) tiles[cnt++] = tile;
            break;
        default:
            break;
        }
    }
    return cnt;
}

// 将牌打表
void map_tiles(const tile_t *tiles, intptr_t cnt, tile_table_t *cnt_table) {
    memset(*cnt_table, 0, sizeof(*cnt_table));
    for (intptr_t i = 0; i < cnt; ++i) {
        ++(*cnt_table)[tiles[i]];
    }
}

// 将手牌打表
bool map_hand_tiles(const hand_tiles_t *hand_tiles, tile_table_t *cnt_table) {
    // 将每一组副露当作3张牌来算，那么总张数=13
    if (hand_tiles->tile_count <= 0 || hand_tiles->pack_count < 0 || hand_tiles->pack_count > 4
        || hand_tiles->pack_count * 3 + hand_tiles->tile_count != 13) {
        return false;
    }

    // 将副露恢复成牌
    tile_t tiles[18];
    intptr_t tile_cnt = 0;
    if (hand_tiles->pack_count == 0) {
        memcpy(tiles, hand_tiles->standing_tiles, 13 * sizeof(tile_t));
        tile_cnt = 13;
    }
    else {
        tile_cnt = packs_to_tiles(hand_tiles->fixed_packs, hand_tiles->pack_count, tiles, 18);
        memcpy(tiles + tile_cnt, hand_tiles->standing_tiles, hand_tiles->tile_count * sizeof(tile_t));
        tile_cnt += hand_tiles->tile_count;
    }

    // 打表
    map_tiles(tiles, tile_cnt, cnt_table);
    return true;
}

// 将表转换成牌
intptr_t table_to_tiles(const tile_table_t &cnt_table, tile_t *tiles, intptr_t max_cnt) {
    intptr_t cnt = 0;
    for (int i = 0; i < 34; ++i) {
        tile_t t = all_tiles[i];
        for (int n = 0; n < cnt_table[t]; ++n) {
            *tiles++ = t;
            ++cnt;
            if (cnt == max_cnt) {
                return max_cnt;
            }
        }
    }
    return cnt;
}

// 计数有效牌枚数
int count_useful_tile(const tile_table_t &used_table, const useful_table_t &useful_table) {
    int cnt = 0;
    for (int i = 0; i < 34; ++i) {
        tile_t t = all_tiles[i];
        if (useful_table[t]) {
            cnt += 4 - used_table[t];
        }
    }
    return cnt;
}

namespace {

    // 路径单元，单元有面子、雀头、搭子等种类，见下面的宏
    // 高8位表示类型，低8位表示牌
    // 对于顺子和顺子搭子，牌指的是最小的一张牌，
    // 例如在顺子123万中，牌为1万，在两面搭子45条中，牌为4条等等
    typedef uint16_t path_unit_t;

#define UNIT_TYPE_CHOW 1                // 顺子
#define UNIT_TYPE_PUNG 2                // 刻子
#define UNIT_TYPE_PAIR 4                // 雀头
#define UNIT_TYPE_CHOW_OPEN_END 5       // 两面或者边张搭子
#define UNIT_TYPE_CHOW_CLOSED 6         // 嵌张搭子
#define UNIT_TYPE_INCOMPLETE_PUNG 7     // 刻子搭子

#define MAKE_UNIT(type_, tile_) static_cast<path_unit_t>(((type_) << 8) | (tile_))
#define UNIT_TYPE(unit_) (((unit_) >> 8) & 0xFF)
#define UNIT_TILE(unit_) ((unit_) & 0xFF)

#define MAX_STATE 512
#define UNIT_SIZE 7

    // 一条路径
    struct work_path_t {
        path_unit_t units[UNIT_SIZE];  // 14/2=7最多7个搭子
        uint16_t depth;  // 当前路径深度
    };

    // 当前工作状态
    struct work_state_t {
        work_path_t paths[MAX_STATE];  // 所有路径
        intptr_t count;  // 路径数量
    };
}

// 路径是否来过了
static bool is_basic_form_branch_exist(const intptr_t fixed_cnt, const work_path_t *work_path, const work_state_t *work_state) {
    if (work_state->count <= 0 || work_path->depth == 0) {
        return false;
    }

    // depth处有信息，所以按stl风格的end应该要+1
    const uint16_t depth = static_cast<uint16_t>(work_path->depth + 1);

    // std::includes要求有序，但又不能破坏当前数据
    work_path_t temp;
    std::copy(&work_path->units[fixed_cnt], &work_path->units[depth], &temp.units[fixed_cnt]);
    std::sort(&temp.units[fixed_cnt], &temp.units[depth]);

    return std::any_of(&work_state->paths[0], &work_state->paths[work_state->count],
        [&temp, fixed_cnt, depth](const work_path_t &path) {
        return std::includes(&path.units[fixed_cnt], &path.units[path.depth], &temp.units[fixed_cnt], &temp.units[depth]);
    });
}

// 保存路径
static void save_work_path(const intptr_t fixed_cnt, const work_path_t *work_path, work_state_t *work_state) {
    if (work_state->count < MAX_STATE) {
        work_path_t &path = work_state->paths[work_state->count++];
        path.depth = work_path->depth;
        std::copy(&work_path->units[fixed_cnt], &work_path->units[work_path->depth], &path.units[fixed_cnt]);

        // 检测是否重复路径时，std::includes要求有序，所以这里将它排序
        std::sort(&path.units[fixed_cnt], &path.units[path.depth]);
    }
    else {
        assert(0 && "too many state!");
    }
}

// 递归计算基本和型上听数
// 参数说明：
//   cnt_table牌表
//   has_pair是否有雀头
//   pack_cnt完成的面子数
//   incomplete_cnt搭子数
// 最后三个参数为优化性能用的，
// work_path保存当前正在计算的路径，
// work_state保存了所有已经计算过的路径，
// 从0到fixed_cnt的数据是不使用的，这些保留给了副露的面子
static int basic_form_shanten_recursively(tile_table_t &cnt_table, const bool has_pair, const unsigned pack_cnt, const unsigned incomplete_cnt,
    const intptr_t fixed_cnt, work_path_t *work_path, work_state_t *work_state) {
    if (pack_cnt == 4) {  // 已经有4组面子
        return has_pair ? -1 : 0;  // 如果有雀头，则和了；如果无雀头，则是听牌
    }

    int max_ret;  // 当前状态能返回的最大上听数

    // 算法说明：
    // 缺少的面子数=4-完成的面子数
    // 缺少的搭子数=缺少的面子数-已有的搭子数
    // 两式合并：缺少的搭子数=4-完成的面子数-已有的搭子数
    int incomplete_need = 4 - pack_cnt - incomplete_cnt;
    if (incomplete_need > 0) {  // 还需要搭子的情况
        // 有雀头时，上听数=已有的搭子数+缺少的搭子数*2-1
        // 无雀头时，上听数=已有的搭子数+缺少的搭子数*2
        max_ret = incomplete_cnt + incomplete_need * 2 - (has_pair ? 1 : 0);
    }
    else {  // 搭子齐了的情况
        // 有雀头时，上听数=3-完成的面子数
        // 无雀头时，上听数=4-完成的面子数
        max_ret = (has_pair ? 3 : 4) - pack_cnt;
    }

    // 当前路径深度
    const unsigned depth = pack_cnt + incomplete_cnt + has_pair;
    work_path->depth = static_cast<uint16_t>(depth);

    int result = max_ret;

    if (pack_cnt + incomplete_cnt > 4) {  // 搭子超载
        save_work_path(fixed_cnt, work_path, work_state);
        return max_ret;
    }

    for (int i = 0; i < 34; ++i) {
        tile_t t = all_tiles[i];
        if (cnt_table[t] < 1) {
            continue;
        }

        // 雀头
        if (!has_pair && cnt_table[t] > 1) {
            work_path->units[depth] = MAKE_UNIT(UNIT_TYPE_PAIR, t);  // 记录雀头
            if (!is_basic_form_branch_exist(fixed_cnt, work_path, work_state)) {
                // 削减雀头，递归
                cnt_table[t] -= 2;
                int ret = basic_form_shanten_recursively(cnt_table, true, pack_cnt, incomplete_cnt,
                    fixed_cnt, work_path, work_state);
                result = std::min(ret, result);
                // 还原
                cnt_table[t] += 2;
            }
        }

        // 刻子
        if (cnt_table[t] > 2) {
            work_path->units[depth] = MAKE_UNIT(UNIT_TYPE_PUNG, t);  // 记录刻子
            if (!is_basic_form_branch_exist(fixed_cnt, work_path, work_state)) {
                // 削减这组刻子，递归
                cnt_table[t] -= 3;
                int ret = basic_form_shanten_recursively(cnt_table, has_pair, pack_cnt + 1, incomplete_cnt,
                    fixed_cnt, work_path, work_state);
                result = std::min(ret, result);
                // 还原
                cnt_table[t] += 3;
            }
        }

        // 顺子（只能是数牌）
        bool is_numbered = is_numbered_suit(t);
        // 顺子t t+1 t+2，显然t不能是8点以上的数牌
        if (is_numbered && tile_get_rank(t) < 8 && cnt_table[t + 1] && cnt_table[t + 2]) {
            work_path->units[depth] = MAKE_UNIT(UNIT_TYPE_CHOW, t);  // 记录顺子
            if (!is_basic_form_branch_exist(fixed_cnt, work_path, work_state)) {
                // 削减这组顺子，递归
                --cnt_table[t];
                --cnt_table[t + 1];
                --cnt_table[t + 2];
                int ret = basic_form_shanten_recursively(cnt_table, has_pair, pack_cnt + 1, incomplete_cnt,
                    fixed_cnt, work_path, work_state);
                result = std::min(ret, result);
                // 还原
                ++cnt_table[t];
                ++cnt_table[t + 1];
                ++cnt_table[t + 2];
            }
        }

        // 如果已经通过削减雀头/面子降低了上听数，再按搭子计算的上听数肯定不会更少
        if (result < max_ret) {
            continue;
        }

        // 刻子搭子
        if (cnt_table[t] > 1) {
            work_path->units[depth] = MAKE_UNIT(UNIT_TYPE_INCOMPLETE_PUNG, t);  // 记录刻子搭子
            if (!is_basic_form_branch_exist(fixed_cnt, work_path, work_state)) {
                // 削减刻子搭子，递归
                cnt_table[t] -= 2;
                int ret = basic_form_shanten_recursively(cnt_table, has_pair, pack_cnt, incomplete_cnt + 1,
                    fixed_cnt, work_path, work_state);
                result = std::min(ret, result);
                // 还原
                cnt_table[t] += 2;
            }
        }

        // 顺子搭子（只能是数牌）
        if (is_numbered) {
            // 两面或者边张搭子t t+1，显然t不能是9点以上的数牌
            if (tile_get_rank(t) < 9 && cnt_table[t + 1]) {  // 两面或者边张
                work_path->units[depth] = MAKE_UNIT(UNIT_TYPE_CHOW_OPEN_END, t);  // 记录两面或者边张搭子
                if (!is_basic_form_branch_exist(fixed_cnt, work_path, work_state)) {
                    // 削减搭子，递归
                    --cnt_table[t];
                    --cnt_table[t + 1];
                    int ret = basic_form_shanten_recursively(cnt_table, has_pair, pack_cnt, incomplete_cnt + 1,
                        fixed_cnt, work_path, work_state);
                    result = std::min(ret, result);
                    // 还原
                    ++cnt_table[t];
                    ++cnt_table[t + 1];
                }
            }
            // 嵌张搭子t t+2，显然t不能是8点以上的数牌
            if (tile_get_rank(t) < 8 && cnt_table[t + 2]) {  // 嵌张
                work_path->units[depth] = MAKE_UNIT(UNIT_TYPE_CHOW_CLOSED, t);  // 记录嵌张搭子
                if (!is_basic_form_branch_exist(fixed_cnt, work_path, work_state)) {
                    // 削减搭子，递归
                    --cnt_table[t];
                    --cnt_table[t + 2];
                    int ret = basic_form_shanten_recursively(cnt_table, has_pair, pack_cnt, incomplete_cnt + 1,
                        fixed_cnt, work_path, work_state);
                    result = std::min(ret, result);
                    // 还原
                    ++cnt_table[t];
                    ++cnt_table[t + 2];
                }
            }
        }
    }

    if (result == max_ret) {
        save_work_path(fixed_cnt, work_path, work_state);
    }

    return result;
}

// 数牌是否有搭子
static bool numbered_tile_has_neighbor(const tile_table_t &cnt_table, tile_t t) {
    rank_t r = tile_get_rank(t);
    if (r < 9) { if (cnt_table[t + 1]) return true; }
    if (r < 8) { if (cnt_table[t + 2]) return true; }
    if (r > 1) { if (cnt_table[t - 1]) return true; }
    if (r > 2) { if (cnt_table[t - 2]) return true; }
    return false;
}

// 以表格为参数计算基本和型上听数
static int basic_form_shanten_from_table(tile_table_t &cnt_table, intptr_t fixed_cnt, useful_table_t *useful_table) {
    // 计算上听数
    work_path_t work_path;
    work_state_t work_state;
    work_state.count = 0;
    int result = basic_form_shanten_recursively(cnt_table, false, static_cast<uint16_t>(fixed_cnt), 0,
        fixed_cnt, &work_path, &work_state);

    if (useful_table == nullptr) {
        return result;
    }

    // 穷举所有的牌，获取能减少上听数的牌
    for (int i = 0; i < 34; ++i) {
        tile_t t = all_tiles[i];
        if (cnt_table[t] == 4 && result > 0) {
            continue;
        }

        if (cnt_table[t] == 0) {
            // 跳过孤张字牌和不靠张的数牌，这些牌都无法减少上听数
            if (is_honor(t) || !numbered_tile_has_neighbor(cnt_table, t)) {
                continue;
            }
        }

        ++cnt_table[t];
        work_state.count = 0;
        int temp = basic_form_shanten_recursively(cnt_table, false, static_cast<uint16_t>(fixed_cnt), 0,
            fixed_cnt, &work_path, &work_state);
        if (temp < result) {
            (*useful_table)[t] = true;  // 标记为有效牌
        }
        --cnt_table[t];
    }

    return result;
}

// 基本和型上听数
int basic_form_shanten(const tile_t *standing_tiles, intptr_t standing_cnt, useful_table_t *useful_table) {
    if (standing_tiles == nullptr || (standing_cnt != 13
        && standing_cnt != 10 && standing_cnt != 7 && standing_cnt != 4 && standing_cnt != 1)) {
        return std::numeric_limits<int>::max();
    }

    // 对立牌的种类进行打表
    tile_table_t cnt_table;
    map_tiles(standing_tiles, standing_cnt, &cnt_table);

    if (useful_table != nullptr) {
        memset(*useful_table, 0, sizeof(*useful_table));
    }
    return basic_form_shanten_from_table(cnt_table, (13 - standing_cnt) / 3, useful_table);
}

// 基本和型判断1张是否听牌
static bool is_basic_form_wait_1(tile_table_t &cnt_table, useful_table_t *waiting_table) {
    for (int i = 0; i < 34; ++i) {
        tile_t t = all_tiles[i];
        if (cnt_table[t] != 1) {
            continue;
        }

        // 单钓将
        cnt_table[t] = 0;
        if (std::all_of(std::begin(cnt_table), std::end(cnt_table), [](int n) { return n == 0; })) {
            cnt_table[t] = 1;
            if (waiting_table != nullptr) {  // 不需要获取听牌张，则可以直接返回
                (*waiting_table)[t] = true;
            }
            return true;
        }
        cnt_table[t] = 1;
    }

    return false;
}

// 基本和型判断2张是否听牌
static bool is_basic_form_wait_2(const tile_table_t &cnt_table, useful_table_t *waiting_table) {
    bool ret = false;
    for (int i = 0; i < 34; ++i) {
        tile_t t = all_tiles[i];
        if (cnt_table[t] < 1) {
            continue;
        }
        if (cnt_table[t] > 1) {
            if (waiting_table != nullptr) {  // 获取听牌张
                (*waiting_table)[t] = true;  // 对倒
                ret = true;
                continue;
            }
            else {  // 不需要获取听牌张，则可以直接返回
                return true;
            }
        }
        if (is_numbered_suit_quick(t)) {  // 数牌搭子
            rank_t r = tile_get_rank(t);
            if (r > 1 && cnt_table[t - 1]) {  // 两面或者边张
                if (waiting_table != nullptr) {  // 获取听牌张
                    if (r < 9) (*waiting_table)[t + 1] = true;
                    if (r > 2) (*waiting_table)[t - 2] = true;
                    ret = true;
                    continue;
                }
                else {  // 不需要获取听牌张，则可以直接返回
                    return true;
                }
            }
            if (r > 2 && cnt_table[t - 2]) {  // 嵌张
                if (waiting_table != nullptr) {  // 获取听牌张
                    (*waiting_table)[t - 1] = true;
                    ret = true;
                    continue;
                }
                else {  // 不需要获取听牌张，则可以直接返回
                    return true;
                }
            }
        }
    }
    return ret;
}

// 基本和型判断4张是否听牌
static bool is_basic_form_wait_4(tile_table_t &cnt_table, useful_table_t *waiting_table) {
    bool ret = false;
    // 削减雀头
    for (int i = 0; i < 34; ++i) {
        tile_t t = all_tiles[i];
        if (cnt_table[t] < 2) {
            continue;
        }
        // 削减雀头，递归
        cnt_table[t] -= 2;
        if (is_basic_form_wait_2(cnt_table, waiting_table)) {
            ret = true;
        }
        // 还原
        cnt_table[t] += 2;
        if (ret && waiting_table == nullptr) {  // 不需要获取听牌张，则可以直接结束递归
            return true;
        }
    }

    return ret;
}

// 递归计算基本和型是否听牌
static bool is_basic_form_wait_recursively(tile_table_t &cnt_table, intptr_t left_cnt, useful_table_t *waiting_table) {
    if (left_cnt == 1) {
        return is_basic_form_wait_1(cnt_table, waiting_table);
    }

    bool ret = false;
    if (left_cnt == 4) {
        ret = is_basic_form_wait_4(cnt_table, waiting_table);
        if (ret && waiting_table == nullptr) {  // 不需要获取听牌张，则可以直接结束递归
            return true;
        }
    }

    for (int i = 0; i < 34; ++i) {
        tile_t t = all_tiles[i];
        if (cnt_table[t] < 1) {
            continue;
        }

        // 刻子
        if (cnt_table[t] > 2) {
            // 削减这组刻子，递归
            cnt_table[t] -= 3;
            if (is_basic_form_wait_recursively(cnt_table, left_cnt - 3, waiting_table)) {
                ret = true;
            }
            // 还原
            cnt_table[t] += 3;
            if (ret && waiting_table == nullptr) {  // 不需要获取听牌张，则可以直接结束递归
                return true;
            }
        }

        // 顺子（只能是数牌）
        if (is_numbered_suit(t)) {
            // 顺子t t+1 t+2，显然t不能是8点以上的数牌
            if (tile_get_rank(t) < 8 && cnt_table[t + 1] && cnt_table[t + 2]) {
                // 削减这组顺子，递归
                --cnt_table[t];
                --cnt_table[t + 1];
                --cnt_table[t + 2];
                if (is_basic_form_wait_recursively(cnt_table, left_cnt - 3, waiting_table)) {
                    ret = true;
                }
                // 还原
                ++cnt_table[t];
                ++cnt_table[t + 1];
                ++cnt_table[t + 2];
                if (ret && waiting_table == nullptr) {  // 不需要获取听牌张，则可以直接结束递归
                    return true;
                }
            }
        }
    }

    return ret;
}

// 基本和型是否听牌
// 这里之所以不用直接调用上听数计算函数，判断其返回值为0的方式
// 是因为前者会削减搭子，这个操作在和牌判断中是没必要的，所以单独写一套更快逻辑
bool is_basic_form_wait(const tile_t *standing_tiles, intptr_t standing_cnt, useful_table_t *waiting_table) {
    // 对立牌的种类进行打表
    tile_table_t cnt_table;
    map_tiles(standing_tiles, standing_cnt, &cnt_table);

    if (waiting_table != nullptr) {
        memset(*waiting_table, 0, sizeof(*waiting_table));
    }
    return is_basic_form_wait_recursively(cnt_table, standing_cnt, waiting_table);
}

// 基本和型2张能否和牌
static bool is_basic_form_win_2(const tile_table_t &cnt_table) {
    // 找到未使用的牌
    typedef std::remove_all_extents<tile_table_t>::type table_elem_t;
    const table_elem_t *it = std::find_if(std::begin(cnt_table), std::end(cnt_table), [](table_elem_t n) { return n > 0; });
    // 存在且张数等于2
    if (it == std::end(cnt_table) || *it != 2) {
        return false;
    }
    // 还有其他未使用的牌
    return std::none_of(it + 1, std::end(cnt_table), [](int n) { return n > 0; });
}

// 递归计算基本和型是否和牌
// 这里之所以不用直接调用上听数计算函数，判断其返回值为-1的方式，
// 是因为前者会削减搭子，这个操作在和牌判断中是没必要的，所以单独写一套更快逻辑
static bool is_basic_form_win_recursively(tile_table_t &cnt_table, intptr_t left_cnt) {
    if (left_cnt == 2) {
        return is_basic_form_win_2(cnt_table);
    }

    for (int i = 0; i < 34; ++i) {
        tile_t t = all_tiles[i];
        if (cnt_table[t] < 1) {
            continue;
        }

        // 刻子
        if (cnt_table[t] > 2) {
            // 削减这组刻子，递归
            cnt_table[t] -= 3;
            bool ret = is_basic_form_win_recursively(cnt_table, left_cnt - 3);
            // 还原
            cnt_table[t] += 3;
            if (ret) {
                return true;
            }
        }

        // 顺子（只能是数牌）
        if (is_numbered_suit(t)) {
            // 顺子t t+1 t+2，显然t不能是8点以上的数牌
            if (tile_get_rank(t) < 8 && cnt_table[t + 1] && cnt_table[t + 2]) {
                // 削减这组顺子，递归
                --cnt_table[t];
                --cnt_table[t + 1];
                --cnt_table[t + 2];
                bool ret = is_basic_form_win_recursively(cnt_table, left_cnt - 3);
                // 还原
                ++cnt_table[t];
                ++cnt_table[t + 1];
                ++cnt_table[t + 2];
                if (ret) {
                    return true;
                }
            }
        }
    }

    return false;
}

// 基本和型是否和牌
bool is_basic_form_win(const tile_t *standing_tiles, intptr_t standing_cnt, tile_t test_tile) {
    // 对立牌的种类进行打表
    tile_table_t cnt_table;
    map_tiles(standing_tiles, standing_cnt, &cnt_table);
    ++cnt_table[test_tile];  // 添加测试的牌
    return is_basic_form_win_recursively(cnt_table, standing_cnt + 1);
}

//-------------------------------- 七对 --------------------------------

// 七对上听数
int seven_pairs_shanten(const tile_t *standing_tiles, intptr_t standing_cnt, useful_table_t *useful_table) {
    if (standing_tiles == nullptr || standing_cnt != 13) {
        return std::numeric_limits<int>::max();
    }

    // 对牌的种类进行打表，并统计对子数
    int pair_cnt = 0;
    tile_table_t cnt_table = { 0 };
    for (intptr_t i = 0; i < standing_cnt; ++i) {
        tile_t tile = standing_tiles[i];
        ++cnt_table[tile];
        if (cnt_table[tile] == 2) {
            ++pair_cnt;
            cnt_table[tile] = 0;
        }
    }

    // 有效牌
    if (useful_table != nullptr) {
        std::transform(std::begin(cnt_table), std::end(cnt_table), std::begin(*useful_table), [](int n) { return n != 0; });
    }
    return 6 - pair_cnt;
}

// 七对是否听牌
bool is_seven_pairs_wait(const tile_t *standing_tiles, intptr_t standing_cnt, useful_table_t *waiting_table) {
    // 直接计算其上听数，上听数为0即为听牌
    if (waiting_table == nullptr) {
        return (0 == seven_pairs_shanten(standing_tiles, standing_cnt, nullptr));
    }

    useful_table_t useful_table;
    if (0 == seven_pairs_shanten(standing_tiles, standing_cnt, &useful_table)) {
        memcpy(*waiting_table, useful_table, sizeof(*waiting_table));
        return true;
    }
    return false;
}

// 七对是否和牌
bool is_seven_pairs_win(const tile_t *standing_tiles, intptr_t standing_cnt, tile_t test_tile) {
    useful_table_t useful_table;
    return (0 == seven_pairs_shanten(standing_tiles, standing_cnt, &useful_table)
        && useful_table[test_tile]);
}

//-------------------------------- 十三幺 --------------------------------

// 十三幺上听数
int thirteen_orphans_shanten(const tile_t *standing_tiles, intptr_t standing_cnt, useful_table_t *useful_table) {
    if (standing_tiles == nullptr || standing_cnt != 13) {
        return std::numeric_limits<int>::max();
    }

    // 对牌的种类进行打表
    tile_table_t cnt_table;
    map_tiles(standing_tiles, standing_cnt, &cnt_table);

    bool has_pair = false;
    int cnt = 0;
    for (int i = 0; i < 13; ++i) {
        int n = cnt_table[standard_thirteen_orphans[i]];
        if (n > 0) {
            ++cnt;  // 幺九牌的种类
            if (n > 1) {
                has_pair = true;  // 幺九牌对子
            }
        }
    }

    // 当有对子时，上听数为：12-幺九牌的种类
    // 当没有对子时，上听数为：13-幺九牌的种类
    int ret = has_pair ? 12 - cnt : 13 - cnt;

    if (useful_table != nullptr) {
        // 先标记所有的幺九牌为有效牌
        memset(*useful_table, 0, sizeof(*useful_table));
        std::for_each(std::begin(standard_thirteen_orphans), std::end(standard_thirteen_orphans),
            [useful_table](tile_t t) {
            (*useful_table)[t] = true;
        });

        // 当有对子时，已有的幺九牌都不需要了
        if (has_pair) {
            for (int i = 0; i < 13; ++i) {
                tile_t t = standard_thirteen_orphans[i];
                int n = cnt_table[t];
                if (n > 0) {
                    (*useful_table)[t] = false;
                }
            }
        }
    }

    return ret;
}

// 十三幺是否听牌
bool is_thirteen_orphans_wait(const tile_t *standing_tiles, intptr_t standing_cnt, useful_table_t *waiting_table) {
    // 直接计算其上听数，上听数为0即为听牌
    if (waiting_table == nullptr) {
        return (0 == thirteen_orphans_shanten(standing_tiles, standing_cnt, nullptr));
    }

    useful_table_t useful_table;
    if (0 == thirteen_orphans_shanten(standing_tiles, standing_cnt, &useful_table)) {
        memcpy(*waiting_table, useful_table, sizeof(*waiting_table));
        return true;
    }
    return false;
}

// 十三幺是否和牌
bool is_thirteen_orphans_win(const tile_t *standing_tiles, intptr_t standing_cnt, tile_t test_tile) {
    useful_table_t useful_table;
    return (0 == thirteen_orphans_shanten(standing_tiles, standing_cnt, &useful_table)
        && useful_table[test_tile]);
}

//-------------------------------- “组合龙+面子+雀头”和型 --------------------------------

// 以表格为参数计算组合龙是否听牌
static bool is_knitted_straight_wait_from_table(const tile_table_t &cnt_table, intptr_t left_cnt, useful_table_t *waiting_table) {
    // 匹配组合龙
    const tile_t (*matched_seq)[9] = nullptr;
    tile_t missing_tiles[9];
    int missing_cnt = 0;
    for (int i = 0; i < 6; ++i) {  // 逐个组合龙测试
        missing_cnt = 0;
        for (int k = 0; k < 9; ++k) {
            tile_t t = standard_knitted_straight[i][k];
            if (cnt_table[t] == 0) {  // 缺失的
                missing_tiles[missing_cnt++] = t;
            }
        }
        if (missing_cnt < 2) {  // 缺2张或以上的肯定没听
            matched_seq = &standard_knitted_straight[i];
            break;
        }
    }

    if (matched_seq == nullptr || missing_cnt > 2) {
        return false;
    }

    // 剔除组合龙
    tile_table_t temp_table;
    memcpy(&temp_table, &cnt_table, sizeof(temp_table));
    for (int i = 0; i < 9; ++i) {
        tile_t t = (*matched_seq)[i];
        if (temp_table[t]) {
            --temp_table[t];
        }
    }

    if (missing_cnt == 1) {  // 如果缺一张，那么除去组合龙之后的牌应该是完成状态才能听牌
        if (left_cnt == 10) {
            if (is_basic_form_win_recursively(temp_table, 2)) {
                if (waiting_table != nullptr) {  // 获取听牌张，听组合龙缺的一张
                    (*waiting_table)[missing_tiles[0]] = true;
                }
                return true;
            }
        }
        else {
            if (is_basic_form_win_recursively(temp_table, 5)) {
                if (waiting_table != nullptr) {  // 获取听牌张，听组合龙缺的一张
                    (*waiting_table)[missing_tiles[0]] = true;
                }
                return true;
            }
        }
    }
    else if (missing_cnt == 0) {  // 如果组合龙齐了，那么除去组合龙之后的牌要能听，整手牌才能听
        if (left_cnt == 10) {
            return is_basic_form_wait_1(temp_table, waiting_table);
        }
        else {
            return is_basic_form_wait_recursively(temp_table, 4, waiting_table);
        }
    }

    return false;
}

// 基本和型包含主番的上听数，可用于计算三步高 三同顺 龙等三组面子的番种整个立牌的上听数
static int basic_form_shanten_specified(const tile_table_t &cnt_table, const tile_t *main_tiles, int main_cnt,
    intptr_t fixed_cnt, useful_table_t *useful_table) {

    tile_table_t temp_table;
    memcpy(&temp_table, &cnt_table, sizeof(temp_table));
    int exist_cnt = 0;

    // 统计主番的牌
    for (int i = 0; i < main_cnt; ++i) {
        tile_t t = main_tiles[i];
        int n = cnt_table[t];
        if (n > 0) {  // 有，削减之
            ++exist_cnt;
            --temp_table[t];
        }
    }

    // 记录有效牌
    if (useful_table != nullptr) {
        memset(*useful_table, 0, sizeof(*useful_table));

        // 统计主番缺失的牌
        for (int i = 0; i < main_cnt; ++i) {
            tile_t t = main_tiles[i];
            int n = cnt_table[t];
            if (n <= 0) {
                (*useful_table)[t] = true;
            }
        }
    }

    // 余下牌的上听数
    int result = basic_form_shanten_from_table(temp_table, fixed_cnt + main_cnt / 3, useful_table);

    // 上听数=主番缺少的张数+余下牌的上听数
    return (main_cnt - exist_cnt) + result;
}

// 组合龙上听数
int knitted_straight_shanten(const tile_t *standing_tiles, intptr_t standing_cnt, useful_table_t *useful_table) {
    if (standing_tiles == nullptr || (standing_cnt != 13 && standing_cnt != 10)) {
        return std::numeric_limits<int>::max();
    }

    // 打表
    tile_table_t cnt_table;
    map_tiles(standing_tiles, standing_cnt, &cnt_table);

    int ret = std::numeric_limits<int>::max();

    // 需要获取有效牌时，计算上听数的同时就获取有效牌了
    if (useful_table != nullptr) {
        memset(*useful_table, 0, sizeof(*useful_table));

        useful_table_t temp_table;

        // 6种组合龙分别计算
        for (int i = 0; i < 6; ++i) {
            int fixed_cnt = (13 - static_cast<int>(standing_cnt)) / 3;
            int st = basic_form_shanten_specified(cnt_table, standard_knitted_straight[i], 9, fixed_cnt, &temp_table);
            if (st < ret) {  // 上听数小的，直接覆盖数据
                ret = st;
                memcpy(*useful_table, temp_table, sizeof(*useful_table));  // 直接覆盖原来的有效牌数据
            }
            else if (st == ret) {  // 两种不同组合龙上听数如果相等的话，直接合并有效牌
                std::transform(std::begin(*useful_table), std::end(*useful_table), std::begin(temp_table),
                    std::begin(*useful_table), [](bool u, bool t) { return u || t; });
            }
        }
    }
    else {
        // 6种组合龙分别计算
        for (int i = 0; i < 6; ++i) {
            int fixed_cnt = (13 - static_cast<int>(standing_cnt)) / 3;
            int st = basic_form_shanten_specified(cnt_table, standard_knitted_straight[i], 9, fixed_cnt, nullptr);
            if (st < ret) {
                ret = st;
            }
        }
    }

    return ret;
}

// 组合龙是否听牌
bool is_knitted_straight_wait(const tile_t *standing_tiles, intptr_t standing_cnt, useful_table_t *waiting_table) {
    if (standing_tiles == nullptr || (standing_cnt != 13 && standing_cnt != 10)) {
        return false;
    }

    // 对立牌的种类进行打表
    tile_table_t cnt_table;
    map_tiles(standing_tiles, standing_cnt, &cnt_table);

    return is_knitted_straight_wait_from_table(cnt_table, standing_cnt, waiting_table);
}

// 组合龙是否和牌
bool is_knitted_straight_win(const tile_t *standing_tiles, intptr_t standing_cnt, tile_t test_tile) {
    useful_table_t waiting_table;
    return (is_knitted_straight_wait(standing_tiles, standing_cnt, &waiting_table)
        && waiting_table[test_tile]);
}

//-------------------------------- 全不靠/七星不靠 --------------------------------

// 1种组合龙的全不靠上听数
static int honors_and_knitted_tiles_shanten_1(const tile_t *standing_tiles, intptr_t standing_cnt, int which_seq, useful_table_t *useful_table) {
    if (standing_tiles == nullptr || standing_cnt != 13) {
        return std::numeric_limits<int>::max();
    }

    // 对牌的种类进行打表
    tile_table_t cnt_table;
    map_tiles(standing_tiles, standing_cnt, &cnt_table);

    int cnt = 0;

    // 统计组合龙部分的数牌
    for (int i = 0; i < 9; ++i) {
        tile_t t = standard_knitted_straight[which_seq][i];
        int n = cnt_table[t];
        if (n > 0) {  // 有，增加计数
            ++cnt;
        }
    }

    // 统计字牌
    for (int i = 6; i < 13; ++i) {
        tile_t t = standard_thirteen_orphans[i];
        int n = cnt_table[t];
        if (n > 0) {  // 有，增加计数
            ++cnt;
        }
    }

    // 记录有效牌
    if (useful_table != nullptr) {
        memset(*useful_table, 0, sizeof(*useful_table));

        // 统计组合龙部分缺失的数牌
        for (int i = 0; i < 9; ++i) {
            tile_t t = standard_knitted_straight[which_seq][i];
            int n = cnt_table[t];
            if (n <= 0) {
                (*useful_table)[t] = true;
            }
        }

        // 统计缺失的字牌
        for (int i = 6; i < 13; ++i) {
            tile_t t = standard_thirteen_orphans[i];
            int n = cnt_table[t];
            if (n <= 0) {
                (*useful_table)[t] = true;
            }
        }
    }

    // 上听数=13-符合牌型的计数
    return 13 - cnt;
}

// 全不靠上听数
int honors_and_knitted_tiles_shanten(const tile_t *standing_tiles, intptr_t standing_cnt, useful_table_t *useful_table) {
    int ret = std::numeric_limits<int>::max();

    // 需要获取有效牌时，计算上听数的同时就获取有效牌了
    if (useful_table != nullptr) {
        memset(*useful_table, 0, sizeof(*useful_table));

        useful_table_t temp_table;

        // 6种组合龙分别计算
        for (int i = 0; i < 6; ++i) {
            int st = honors_and_knitted_tiles_shanten_1(standing_tiles, standing_cnt, i, &temp_table);
            if (st < ret) {  // 上听数小的，直接覆盖数据
                ret = st;
                memcpy(*useful_table, temp_table, sizeof(*useful_table));  // 直接覆盖原来的有效牌数据
            }
            else if (st == ret) {  // 两种不同组合龙上听数如果相等的话，直接合并有效牌
                std::transform(std::begin(*useful_table), std::end(*useful_table), std::begin(temp_table),
                    std::begin(*useful_table), [](bool u, bool t) { return u || t; });
            }
        }
    }
    else {
        // 6种组合龙分别计算
        for (int i = 0; i < 6; ++i) {
            int st = honors_and_knitted_tiles_shanten_1(standing_tiles, standing_cnt, i, nullptr);
            if (st < ret) {
                ret = st;
            }
        }
    }
    return ret;
}

// 全不靠是否听牌
bool is_honors_and_knitted_tiles_wait(const tile_t *standing_tiles, intptr_t standing_cnt, useful_table_t *waiting_table) {
    // 直接计算其上听数，上听数为0即为听牌
    if (waiting_table == nullptr) {
        return (0 == honors_and_knitted_tiles_shanten(standing_tiles, standing_cnt, nullptr));
    }

    useful_table_t useful_table;
    if (0 == honors_and_knitted_tiles_shanten(standing_tiles, standing_cnt, &useful_table)) {
        memcpy(*waiting_table, useful_table, sizeof(*waiting_table));
        return true;
    }
    return false;
}

// 全不靠是否和牌
bool is_honors_and_knitted_tiles_win(const tile_t *standing_tiles, intptr_t standing_cnt, tile_t test_tile) {
    useful_table_t useful_table;
    if (0 == honors_and_knitted_tiles_shanten(standing_tiles, standing_cnt, &useful_table)) {
        return useful_table[test_tile];
    }
    return false;
}

//-------------------------------- 枚举打牌 --------------------------------

// 枚举打哪张牌1次
static bool enum_discard_tile_1(const hand_tiles_t *hand_tiles, tile_t discard_tile, uint8_t form_flag,
    void *context, enum_callback_t enum_callback) {
    enum_result_t result;
    result.discard_tile = discard_tile;
    result.form_flag = FORM_FLAG_BASIC_FORM;
    result.shanten = basic_form_shanten(hand_tiles->standing_tiles, hand_tiles->tile_count, &result.useful_table);
    if (result.shanten == 0 && result.useful_table[discard_tile]) {  // 0上听，并且打出的牌是有效牌，则修正为和了
        result.shanten = -1;
    }
    if (!enum_callback(context, &result)) {
        return false;
    }

    // 立牌有13张时，才需要计算特殊和型
    if (hand_tiles->tile_count == 13) {
        if (form_flag | FORM_FLAG_SEVEN_PAIRS) {
            result.form_flag = FORM_FLAG_SEVEN_PAIRS;
            result.shanten = seven_pairs_shanten(hand_tiles->standing_tiles, hand_tiles->tile_count, &result.useful_table);
            if (result.shanten == 0 && result.useful_table[discard_tile]) {  // 0上听，并且打出的牌是有效牌，则修正为和了
                result.shanten = -1;
            }
            if (!enum_callback(context, &result)) {
                return false;
            }
        }

        if (form_flag | FORM_FLAG_THIRTEEN_ORPHANS) {
            result.form_flag = FORM_FLAG_THIRTEEN_ORPHANS;
            result.shanten = thirteen_orphans_shanten(hand_tiles->standing_tiles, hand_tiles->tile_count, &result.useful_table);
            if (result.shanten == 0 && result.useful_table[discard_tile]) {  // 0上听，并且打出的牌是有效牌，则修正为和了
                result.shanten = -1;
            }
            if (!enum_callback(context, &result)) {
                return false;
            }
        }

        if (form_flag | FORM_FLAG_HONORS_AND_KNITTED_TILES) {
            result.form_flag = FORM_FLAG_HONORS_AND_KNITTED_TILES;
            result.shanten = honors_and_knitted_tiles_shanten(hand_tiles->standing_tiles, hand_tiles->tile_count, &result.useful_table);
            if (result.shanten == 0 && result.useful_table[discard_tile]) {  // 0上听，并且打出的牌是有效牌，则修正为和了
                result.shanten = -1;
            }
            if (!enum_callback(context, &result)) {
                return false;
            }
        }
    }

    // 立牌有13张或者10张时，才需要计算组合龙
    if (hand_tiles->tile_count == 13 || hand_tiles->tile_count == 10) {
        if (form_flag | FORM_FLAG_KNITTED_STRAIGHT) {
            result.form_flag = FORM_FLAG_KNITTED_STRAIGHT;
            result.shanten = knitted_straight_shanten(hand_tiles->standing_tiles, hand_tiles->tile_count, &result.useful_table);
            if (result.shanten == 0 && result.useful_table[discard_tile]) {  // 0上听，并且打出的牌是有效牌，则修正为和了
                result.shanten = -1;
            }
            if (!enum_callback(context, &result)) {
                return false;
            }
        }
    }

    return true;
}

// 枚举打哪张牌
void enum_discard_tile(const hand_tiles_t *hand_tiles, tile_t serving_tile, uint8_t form_flag,
    void *context, enum_callback_t enum_callback) {
    // 先计算摸切的
    if (!enum_discard_tile_1(hand_tiles, serving_tile, form_flag, context, enum_callback)) {
        return;
    }

    if (serving_tile == 0) {
        return;
    }

    // 将立牌打表
    tile_table_t cnt_table;
    map_tiles(hand_tiles->standing_tiles, hand_tiles->tile_count, &cnt_table);

    // 复制一份手牌
    hand_tiles_t temp;
    memcpy(&temp, hand_tiles, sizeof(temp));

    // 依次尝试打手中的立牌
    for (int i = 0; i < 34; ++i) {
        tile_t t = all_tiles[i];
        if (cnt_table[t] && t != serving_tile && cnt_table[serving_tile] < 4) {
            --cnt_table[t];  // 打这张牌
            ++cnt_table[serving_tile];  // 上这张牌

            // 从table转成立牌
            table_to_tiles(cnt_table, temp.standing_tiles, temp.tile_count);

            // 计算
            if (!enum_discard_tile_1(&temp, t, form_flag, context, enum_callback)) {
                return;
            }

            // 复原
            --cnt_table[serving_tile];
            ++cnt_table[t];
        }
    }
}

}

/**
 * 算番流程概述：
 * 1. 判断特殊和型
 *   (1) 门清状态——七对、十三幺、组合龙、全不靠
 *   (2) 1副露状态——组合龙
 * 2. 按基本和型划分（留意七对）
 * 3. 对第2步中的划分结果进行算番，取最大值

 * 对划分后的结果算番流程：
 * 1. 计数顺子个数和刻子个数，对刻子自身算番（涉及：幺九刻、箭刻）
 * 2. 分情况对面子算番
 *   (1) 4顺——先判断三色/一色双龙会，没有再计算4顺的番
 *   (2) 3顺1刻——计算3顺的番
 *   (3) 2顺2刻——计算2顺的番，计算2刻的番
 *   (4) 1顺3刻——计算3刻的番
 * 3. 九莲宝灯判断
 * 4. 根据和牌方式调整——涉及：不求人、全求人
 * 5. 根据雀头调整——涉及：平和、小三元、小四喜
 * 6. 根据牌组特征调整——涉及：全带幺、全带五、全双刻
 * 7. 根据花色调整——涉及：无字、缺一门、混一色、清一色、五门齐
 * 8. 根据牌特性调整——涉及：断幺、推不倒、绿一色、字一色、清幺九、混幺九
 * 9. 根据数牌的范围调整——涉及：大于五、小于五、全大、全中、全小
 * 10. 计算四归一
 * 11. 根据听牌方式调整——涉及：边张、嵌张、单钓将
 * 12. 根据风调整——涉及：圈风刻、门风刻
 * 13. 最后统一调整规则中规定不计的，得到算番结果。如果为0番，则调整为无番和
 */

#define MAX_DIVISION_CNT 20  // 一副牌最多也没有20种划分吧，够用了

#if 0
#define LOG(fmt_, ...) printf(fmt_, ##__VA_ARGS__)
#else
#define LOG(...) do { } while (0)
#endif

//#define STRICT_98_RULE

namespace mahjong {

#if 0  // Debug
extern intptr_t packs_to_string(const pack_t *packs, intptr_t pack_cnt, char *str, intptr_t max_size);
#endif

//-------------------------------- 划分 --------------------------------

namespace {

    // 划分
    struct division_t {
        pack_t packs[5];  // 牌组。4面子1雀头，共5组
    };

    // 划分结果
    struct division_result_t {
        division_t divisions[MAX_DIVISION_CNT];  // 每一种划分
        intptr_t count;  // 划分方式总数
    };
}

// 递归划分算法的最后一步，添加划分
static void divide_tail_add_division(intptr_t fixed_cnt, const division_t *work_division, division_result_t *result) {
    // 拷贝一份当前的划分出来的面子，并排序暗手的面子
    // 这里不能直接在work_division->packs上排序，否则会破坏递归外层的数据
    division_t temp;
    memcpy(&temp, work_division, sizeof(temp));
    std::sort(temp.packs + fixed_cnt, temp.packs + 4);

    // 如果这种划分是否已经存在了
    if (std::none_of(&result->divisions[0], &result->divisions[result->count],
        [&temp, fixed_cnt](const division_t &od) {
        return std::equal(&od.packs[fixed_cnt], &od.packs[4], &temp.packs[fixed_cnt]);
    })) {
        // 写入划分结果里
        memcpy(&result->divisions[result->count], &temp, sizeof(temp));
        ++result->count;
    }
    else {
        LOG("same case");
    }
}

// 递归划分的最后一步
static bool divide_tail(tile_table_t &cnt_table, intptr_t fixed_cnt, division_t *work_division, division_result_t *result) {
    for (int i = 0; i < 34; ++i) {
        tile_t t = all_tiles[i];
        if (cnt_table[t] < 2) {
            continue;
        }

        cnt_table[t] -= 2;  // 削减
        // 所有牌全部使用完毕
        if (std::all_of(std::begin(cnt_table), std::end(cnt_table), [](int n) { return n == 0; })) {
            cnt_table[t] += 2;  // 还原

            // 这2张作为雀头
            work_division->packs[4] = make_pack(0, PACK_TYPE_PAIR, t);
            divide_tail_add_division(fixed_cnt, work_division, result);  // 记录
            return true;
        }
        cnt_table[t] += 2;  // 还原
    }

    return false;
}

// 判断一条划分分支是否来过
static bool is_division_branch_exist(intptr_t fixed_cnt, intptr_t step, const division_t *work_division, const division_result_t *result) {
    // 没有划分时，以及划分步骤小于3时，不检测，因为至少要有3步递归才会产生不同划分
    if (result->count <= 0 || step < 3) {
        return false;
    }

    // std::includes要求有序
    // 这里不能直接在work_division->packs上排序，否则会破坏递归外层的数据
    division_t temp;
    memcpy(&temp.packs[fixed_cnt], &work_division->packs[fixed_cnt], step * sizeof(pack_t));
    std::sort(&temp.packs[fixed_cnt], &temp.packs[fixed_cnt + step]);

    // 只需要比较面子是否重复分支，雀头不参与比较，所以下标是4
    return std::any_of(&result->divisions[0], &result->divisions[result->count],
        [&temp, fixed_cnt, step](const division_t &od) {
        return std::includes(&od.packs[fixed_cnt], &od.packs[4], &temp.packs[fixed_cnt], &temp.packs[fixed_cnt + step]);
    });
}

// 递归划分
static bool divide_recursively(tile_table_t &cnt_table, intptr_t fixed_cnt, intptr_t step, division_t *work_division, division_result_t *result) {
    const intptr_t idx = step + fixed_cnt;
    if (idx == 4) {  // 4组面子都有了
        return divide_tail(cnt_table, fixed_cnt, work_division, result);
    }

    bool ret = false;

    // 按牌表张遍历牌
    for (int i = 0; i < 34; ++i) {
        tile_t t = all_tiles[i];
        if (cnt_table[t] < 1) {
            continue;
        }

        // 刻子
        if (cnt_table[t] > 2) {
            work_division->packs[idx] = make_pack(0, PACK_TYPE_PUNG, t);  // 记录刻子
            if (!is_division_branch_exist(fixed_cnt, step + 1, work_division, result)) {
                // 削减这组刻子，递归
                cnt_table[t] -= 3;
                if (divide_recursively(cnt_table, fixed_cnt, step + 1, work_division, result)) {
                    ret = true;
                }
                // 还原
                cnt_table[t] += 3;
            }
        }

        // 顺子（只能是数牌）
        if (is_numbered_suit(t)) {
            if (tile_get_rank(t) < 8 && cnt_table[t + 1] && cnt_table[t + 2]) {
                work_division->packs[idx] = make_pack(0, PACK_TYPE_CHOW, static_cast<tile_t>(t + 1));  // 记录顺子
                if (!is_division_branch_exist(fixed_cnt, step + 1, work_division, result)) {
                    // 削减这组顺子，递归
                    --cnt_table[t];
                    --cnt_table[t + 1];
                    --cnt_table[t + 2];
                    if (divide_recursively(cnt_table, fixed_cnt, step + 1, work_division, result)) {
                        ret = true;
                    }
                    // 还原
                    ++cnt_table[t];
                    ++cnt_table[t + 1];
                    ++cnt_table[t + 2];
                }
            }
        }
    }

    return ret;
}

// 划分一手牌
static bool divide_win_hand(const tile_t *standing_tiles, const pack_t *fixed_packs, intptr_t fixed_cnt, division_result_t *result) {
    intptr_t standing_cnt = 14 - fixed_cnt * 3;

    // 对立牌的种类进行打表
    tile_table_t cnt_table;
    map_tiles(standing_tiles, standing_cnt, &cnt_table);

    result->count = 0;

    // 复制副露的面子
    division_t work_division;
    memcpy(work_division.packs, fixed_packs, fixed_cnt * sizeof(pack_t));
    return divide_recursively(cnt_table, fixed_cnt, 0, &work_division, result);
}

//-------------------------------- 算番 --------------------------------

// 4组递增1
static forceinline bool is_four_shifted_1(rank_t r0, rank_t r1, rank_t r2, rank_t r3) {
    return (r0 + 1 == r1 && r1 + 1 == r2 && r2 + 1 == r3);
}

// 4组递增2
static forceinline bool is_four_shifted_2(rank_t r0, rank_t r1, rank_t r2, rank_t r3) {
    return (r0 + 2 == r1 && r1 + 2 == r2 && r2 + 2 == r3);
}

// 3组递增1
static forceinline bool is_shifted_1(rank_t r0, rank_t r1, rank_t r2) {
    return (r0 + 1 == r1 && r1 + 1 == r2);
}

// 3组递增2
static forceinline bool is_shifted_2(rank_t r0, rank_t r1, rank_t r2) {
    return (r0 + 2 == r1 && r1 + 2 == r2);
}

// 三色
static forceinline bool is_mixed(suit_t s0, suit_t s1, suit_t s2) {
    return (s0 != s1 && s0 != s2 && s1 != s2);
}

// 3组递增1无序
static forceinline bool is_shifted_1_unordered(rank_t r0, rank_t r1, rank_t r2) {
    return is_shifted_1(r1, r0, r2) || is_shifted_1(r2, r0, r1) || is_shifted_1(r0, r1, r2)
        || is_shifted_1(r2, r1, r0) || is_shifted_1(r0, r2, r1) || is_shifted_1(r1, r2, r0);
}

// 4组顺子的番
static fan_t get_4_chows_fan(tile_t t0, tile_t t1, tile_t t2, tile_t t3) {
    // 按出现频率顺序

    // 一色四步高
    if (is_four_shifted_2(t0, t1, t2, t3) || is_four_shifted_1(t0, t1, t2, t3)) {
        return FOUR_PURE_SHIFTED_CHOWS;
    }
    // 一色四同顺
    if (t0 == t1 && t0 == t2 && t0 == t3) {
        return QUADRUPLE_CHOW;
    }
    // 以上都没有
    return FAN_NONE;
}

// 3组顺子的番
static fan_t get_3_chows_fan(tile_t t0, tile_t t1, tile_t t2) {
    suit_t s0 = tile_get_suit(t0);
    suit_t s1 = tile_get_suit(t1);
    suit_t s2 = tile_get_suit(t2);

    rank_t r0 = tile_get_rank(t0);
    rank_t r1 = tile_get_rank(t1);
    rank_t r2 = tile_get_rank(t2);

    // 按出现频率顺序

    if (is_mixed(s0, s1, s2)) {  // 三色
        // 三色三步高
        if (is_shifted_1_unordered(r1, r0, r2)) {
            return MIXED_SHIFTED_CHOWS;
        }
        // 三色三同顺
        if (r0 == r1 && r1 == r2) {
            return MIXED_TRIPLE_CHOW;
        }
        // 花龙
        if ((r0 == 2 && r1 == 5 && r2 == 8) || (r0 == 2 && r1 == 8 && r2 == 5)
            || (r0 == 5 && r1 == 2 && r2 == 8) || (r0 == 5 && r1 == 8 && r2 == 2)
            || (r0 == 8 && r1 == 2 && r2 == 5) || (r0 == 8 && r1 == 5 && r2 == 2)) {
            return MIXED_STRAIGHT;
        }
    }
    else {  // 一色
        // 清龙
        if (t0 + 3 == t1 && t1 + 3 == t2) {
            return PURE_STRAIGHT;
        }
        // 一色三步高
        if (is_shifted_2(t0, t1, t2) || is_shifted_1(t0, t1, t2)) {
            return PURE_SHIFTED_CHOWS;
        }
        // 一色三同顺
        if (t0 == t1 && t0 == t2) {
            return PURE_TRIPLE_CHOW;
        }
    }
    // 以上都没有
    return FAN_NONE;
}

// 2组顺子的番
static fan_t get_2_chows_fan_unordered(tile_t t0, tile_t t1) {
    // 按出现频率顺序

    if (!is_suit_equal_quick(t0, t1)) {  // 两色
        // 喜相逢
        if (is_rank_equal_quick(t0, t1)) {
            return MIXED_DOUBLE_CHOW;
        }
    }
    else {  // 一色
        // 连六
        if (t0 + 3 == t1 || t1 + 3 == t0) {
            return SHORT_STRAIGHT;
        }
        // 老少副
        rank_t r0 = tile_get_rank(t0);
        rank_t r1 = tile_get_rank(t1);
        if ((r0 == 2 && r1 == 8) || (r0 == 8 && r1 == 2)) {
            return TWO_TERMINAL_CHOWS;
        }
        // 一般高
        if (t0 == t1) {
            return PURE_DOUBLE_CHOW;
        }
    }
    // 以上都没有
    return FAN_NONE;
}

// 4组刻子的番
static fan_t get_4_pungs_fan(tile_t t0, tile_t t1, tile_t t2, tile_t t3) {
    // 一色四节高
    if (is_numbered_suit_quick(t0) && t0 + 1 == t1 && t1 + 1 == t2 && t2 + 1 == t3) {
        return FOUR_PURE_SHIFTED_PUNGS;
    }
    // 大四喜
    if (t0 == TILE_E && t1 == TILE_S && t2 == TILE_W && t3 == TILE_N) {
        return BIG_FOUR_WINDS;
    }
    // 以上都没有
    return FAN_NONE;
}

// 3组刻子的番
static fan_t get_3_pungs_fan(tile_t t0, tile_t t1, tile_t t2) {
    // 按出现频率顺序

    if (is_numbered_suit_quick(t0) && is_numbered_suit_quick(t1) && is_numbered_suit_quick(t2)) {  // 数牌
        suit_t s0 = tile_get_suit(t0);
        suit_t s1 = tile_get_suit(t1);
        suit_t s2 = tile_get_suit(t2);

        rank_t r0 = tile_get_rank(t0);
        rank_t r1 = tile_get_rank(t1);
        rank_t r2 = tile_get_rank(t2);

        if (is_mixed(s0, s1, s2)) {  // 三色
            // 三色三节高
            if (is_shifted_1_unordered(r1, r0, r2)) {
                return MIXED_SHIFTED_PUNGS;
            }
            // 三同刻
            if (r0 == r1 && r1 == r2) {
                return TRIPLE_PUNG;
            }
        }
        else {
            // 一色三节高
            if (t0 + 1 == t1 && t1 + 1 == t2) {
                return PURE_SHIFTED_PUNGS;
            }
        }
    }
    else {
        // 三风刻
        if ((t0 == TILE_E && t1 == TILE_S && t2 == TILE_W)
            || (t0 == TILE_E && t1 == TILE_S && t2 == TILE_N)
            || (t0 == TILE_E && t1 == TILE_W && t2 == TILE_N)
            || (t0 == TILE_S && t1 == TILE_W && t2 == TILE_N)) {
            return BIG_THREE_WINDS;
        }
        // 大三元
        if (t0 == TILE_C && t1 == TILE_F && t2 == TILE_P) {
            return BIG_THREE_DRAGONS;
        }
    }

    // 以上都没有
    return FAN_NONE;
}

// 2组刻子的番
static fan_t get_2_pungs_fan_unordered(tile_t t0, tile_t t1) {
    // 按出现频率顺序
    if (is_numbered_suit_quick(t0) && is_numbered_suit_quick(t1)) {  // 数牌
        // 双同刻
        if (is_rank_equal_quick(t0, t1)) {
            return DOUBLE_PUNG;
        }
    }
    else {
        // 双箭刻
        if (is_dragons(t0) && is_dragons(t1)) {
            return TWO_DRAGONS_PUNGS;
        }
    }
    // 以上都没有
    return FAN_NONE;
}

// 1组刻子的番
static fan_t get_1_pung_fan(tile_t mid_tile) {
    // 箭刻
    if (is_dragons(mid_tile)) {
        return DRAGON_PUNG;
    }
    // 幺九刻
    if (is_terminal(mid_tile) || is_winds(mid_tile)) {
        return PUNG_OF_TERMINALS_OR_HONORS;
    }
    // 以上都没有
    return FAN_NONE;
}

// 存在3组顺子的番种时，余下的第4组顺子最多算1番
static fan_t get_1_chow_extra_fan(tile_t tile0, tile_t tile1, tile_t tile2, tile_t tile_extra) {
    fan_t fan0 = get_2_chows_fan_unordered(tile0, tile_extra);
    fan_t fan1 = get_2_chows_fan_unordered(tile1, tile_extra);
    fan_t fan2 = get_2_chows_fan_unordered(tile2, tile_extra);

    // 按以下顺序返回
    // 一般高
    if (fan0 == PURE_DOUBLE_CHOW || fan1 == PURE_DOUBLE_CHOW || fan2 == PURE_DOUBLE_CHOW) {
        return PURE_DOUBLE_CHOW;
    }
    // 喜相逢
    if (fan0 == MIXED_DOUBLE_CHOW || fan1 == MIXED_DOUBLE_CHOW || fan2 == MIXED_DOUBLE_CHOW) {
        return MIXED_DOUBLE_CHOW;
    }
    // 连六
    if (fan0 == SHORT_STRAIGHT || fan1 == SHORT_STRAIGHT || fan2 == SHORT_STRAIGHT) {
        return SHORT_STRAIGHT;
    }
    // 老少副
    if (fan0 == TWO_TERMINAL_CHOWS || fan1 == TWO_TERMINAL_CHOWS || fan2 == TWO_TERMINAL_CHOWS) {
        return TWO_TERMINAL_CHOWS;
    }

    return FAN_NONE;
}

// 套算一次原则：
// 如果有尚未组合过的一副牌，只可同已组合过的相应的一副牌套算一次
//
// 不得相同原则：
// 凡已经合过某一番种的牌，不能再同其他一副牌组成相同的番种计分
//
// 根据套算一次原则，234567s234567p，只能计为“喜相逢*2 连六*1”或者“喜相逢*1 连六*2”，而不是“喜相逢*2 连六*2”
// 根据以上两点，234s223344567p，只能计为：“一般高、喜相逢、连六”，而不是“喜相逢*2、连六”
//
// 直接按规则来写，差不多是图的算法，太过复杂
// 这里简便处理：先统计有多少番，当超过时规则允许的数目时，从重复的开始削减
static void exclusionary_rule(const fan_t *all_fans, long fan_cnt, long max_cnt, fan_table_t &fan_table) {
    // 统计有多少番
    uint16_t table[4] = { 0 };
    long cnt = 0;
    for (long i = 0; i < fan_cnt; ++i) {
        if (all_fans[i] != FAN_NONE) {
            ++cnt;
            ++table[all_fans[i] - PURE_DOUBLE_CHOW];
        }
    }

    // 当超过时，从重复的开始削减
    int limit_cnt = 1;
    // 第一轮先削减剩下1，第二轮削减剩下0
    while (cnt > max_cnt && limit_cnt >= 0) {
        int idx = 4;  // 从老少副开始削减
        while (cnt > max_cnt && idx-- > 0) {
            while (static_cast<int>(table[idx]) > limit_cnt && cnt > max_cnt) {
                --table[idx];
                --cnt;
            }
        }
        --limit_cnt;
    }

    fan_table[PURE_DOUBLE_CHOW] = table[0];
    fan_table[MIXED_DOUBLE_CHOW] = table[1];
    fan_table[SHORT_STRAIGHT] = table[2];
    fan_table[TWO_TERMINAL_CHOWS] = table[3];
}

// 4组顺子算番
static void calculate_4_chows(const tile_t (&mid_tiles)[4], fan_table_t &fan_table) {
    fan_t fan;
    // 存在4组顺子的番种时，不再检测其他的了
    if ((fan = get_4_chows_fan(mid_tiles[0], mid_tiles[1], mid_tiles[2], mid_tiles[3])) != FAN_NONE) {
        fan_table[fan] = 1;
        return;
    }

    // 3组顺子判断
    // 012构成3组顺子的番种
    if ((fan = get_3_chows_fan(mid_tiles[0], mid_tiles[1], mid_tiles[2])) != FAN_NONE) {
        fan_table[fan] = 1;
        // 计算与第4组顺子构成的番
        if ((fan = get_1_chow_extra_fan(mid_tiles[0], mid_tiles[1], mid_tiles[2], mid_tiles[3])) != FAN_NONE) {
            fan_table[fan] = 1;
        }
        return;
    }
    // 013构成3组顺子的番种
    else if ((fan = get_3_chows_fan(mid_tiles[0], mid_tiles[1], mid_tiles[3])) != FAN_NONE) {
        fan_table[fan] = 1;
        // 计算与第4组顺子构成的番
        if ((fan = get_1_chow_extra_fan(mid_tiles[0], mid_tiles[1], mid_tiles[3], mid_tiles[2])) != FAN_NONE) {
            fan_table[fan] = 1;
        }
        return;
    }
    // 023构成3组顺子的番种
    else if ((fan = get_3_chows_fan(mid_tiles[0], mid_tiles[2], mid_tiles[3])) != FAN_NONE) {
        fan_table[fan] = 1;
        // 计算与第4组顺子构成的番
        if ((fan = get_1_chow_extra_fan(mid_tiles[0], mid_tiles[2], mid_tiles[3], mid_tiles[1])) != FAN_NONE) {
            fan_table[fan] = 1;
        }
        return;
    }
    // 123构成3组顺子的番种
    else if ((fan = get_3_chows_fan(mid_tiles[1], mid_tiles[2], mid_tiles[3])) != FAN_NONE) {
        fan_table[fan] = 1;
        // 计算与第4组顺子构成的番
        if ((fan = get_1_chow_extra_fan(mid_tiles[1], mid_tiles[2], mid_tiles[3], mid_tiles[0])) != FAN_NONE) {
            fan_table[fan] = 1;
        }
        return;
    }

    // 不存在3组顺子的番种时，4组顺子最多3番
    fan_t all_fans[6] = {
        get_2_chows_fan_unordered(mid_tiles[0], mid_tiles[1]),
        get_2_chows_fan_unordered(mid_tiles[0], mid_tiles[2]),
        get_2_chows_fan_unordered(mid_tiles[0], mid_tiles[3]),
        get_2_chows_fan_unordered(mid_tiles[1], mid_tiles[2]),
        get_2_chows_fan_unordered(mid_tiles[1], mid_tiles[3]),
        get_2_chows_fan_unordered(mid_tiles[2], mid_tiles[3])
    };

    int max_cnt = 3;

    // 0与其他3组顺子无任何关系
    if (all_fans[0] == FAN_NONE && all_fans[1] == FAN_NONE && all_fans[2] == FAN_NONE) {
        --max_cnt;
    }

    // 1与其他3组顺子无任何关系
    if (all_fans[0] == FAN_NONE && all_fans[3] == FAN_NONE && all_fans[4] == FAN_NONE) {
        --max_cnt;
    }

    // 2与其他3组顺子无任何关系
    if (all_fans[1] == FAN_NONE && all_fans[3] == FAN_NONE && all_fans[5] == FAN_NONE) {
        --max_cnt;
    }

    // 3与其他3组顺子无任何关系
    if (all_fans[2] == FAN_NONE && all_fans[4] == FAN_NONE && all_fans[5] == FAN_NONE) {
        --max_cnt;
    }

    if (max_cnt > 0) {
        exclusionary_rule(all_fans, 6, max_cnt, fan_table);
    }
}

// 3组顺子算番
static void calculate_3_chows(const tile_t (&mid_tiles)[3], fan_table_t &fan_table) {
    fan_t fan;

    // 存在3组顺子的番种时，不再检测其他的
    if ((fan = get_3_chows_fan(mid_tiles[0], mid_tiles[1], mid_tiles[2])) != FAN_NONE) {
        fan_table[fan] = 1;
        return;
    }

    // 不存在上述番种时，3组顺子最多2番
    fan_t all_fans[3] = {
        get_2_chows_fan_unordered(mid_tiles[0], mid_tiles[1]),
        get_2_chows_fan_unordered(mid_tiles[0], mid_tiles[2]),
        get_2_chows_fan_unordered(mid_tiles[1], mid_tiles[2])
    };
    exclusionary_rule(all_fans, 3, 2, fan_table);
}

// 2组顺子算番
static void calculate_2_chows_unordered(const tile_t (&mid_tiles)[2], fan_table_t &fan_table) {
    fan_t fan;
    if ((fan = get_2_chows_fan_unordered(mid_tiles[0], mid_tiles[1])) != FAN_NONE) {
        ++fan_table[fan];
    }
}

// 刻子（杠）算番
static void calculate_kongs(const pack_t *pung_packs, intptr_t pung_cnt, fan_table_t &fan_table) {
    // 统计明杠 暗杠 明刻 暗刻
    int melded_kong_cnt = 0;
    int concealed_kong_cnt = 0;
    int concealed_pung_cnt = 0;
    for (intptr_t i = 0; i < pung_cnt; ++i) {
        if (is_pack_melded(pung_packs[i])) {
            if (pack_get_type(pung_packs[i]) == PACK_TYPE_KONG) {
                ++melded_kong_cnt;
            }
        }
        else {
            if (pack_get_type(pung_packs[i]) == PACK_TYPE_KONG) {
                ++concealed_kong_cnt;
            }
            else {
                ++concealed_pung_cnt;
            }
        }
    }

    // 规则
    // 三杠
    // 明杠 明杠 暗杠 暗刻 -> 三杠+双暗刻+碰碰和
    // 明杠 暗杠 暗杠 明刻 -> 三杠+双暗刻+碰碰和
    // 明杠 暗杠 暗杠 暗刻 -> 三杠+三暗刻+碰碰和
    // 暗杠 暗杠 暗杠 明刻 -> 三杠+三暗刻+碰碰和
    // 暗杠 暗杠 暗杠 暗刻 -> 三杠+四暗刻
    //
    // 四杠
    // 暗杠 明杠 明杠 明杠 -> 四杠
    // 暗杠 暗杠 明杠 明杠 -> 四杠+双暗刻
    // 暗杠 暗杠 暗杠 明杠 -> 四杠+三暗刻
    // 暗杠 暗杠 暗杠 暗杠 -> 四杠+四暗刻
    //

    int kong_cnt = melded_kong_cnt + concealed_kong_cnt;
    switch (kong_cnt) {
    case 0:  // 0个杠
        switch (concealed_pung_cnt) {  // 暗刻的个数
        case 2: fan_table[TWO_CONCEALED_PUNGS] = 1; break;
        case 3: fan_table[THREE_CONCEALED_PUNGS] = 1; break;
        case 4: fan_table[FOUR_CONCEALED_PUNGS] = 1; break;
        default: break;
        }
        break;
    case 1:  // 1个杠
        if (melded_kong_cnt == 1) {  // 明杠
            fan_table[MELDED_KONG] = 1;
            switch (concealed_pung_cnt) {  // 暗刻的个数
            case 2: fan_table[TWO_CONCEALED_PUNGS] = 1; break;
            case 3: fan_table[THREE_CONCEALED_PUNGS] = 1; break;
            default: break;
            }
        }
        else {  // 暗杠
            fan_table[CONCEALED_KONG] = 1;
            switch (concealed_pung_cnt) {  // 暗刻的个数
            case 1: fan_table[TWO_CONCEALED_PUNGS] = 1; break;
            case 2: fan_table[THREE_CONCEALED_PUNGS] = 1; break;
            case 3: fan_table[FOUR_CONCEALED_PUNGS] = 1; break;
            default: break;
            }
        }
        break;
    case 2:  // 2个杠
        switch (concealed_kong_cnt) {
        case 0:  // 双明杠
            fan_table[TWO_MELDED_KONGS] = 1;
            switch (concealed_pung_cnt) {  // 暗刻的个数
            case 2: fan_table[TWO_CONCEALED_PUNGS] = 1; break;
            default: break;
            }
            break;
        case 1:  // 明暗杠
#if SUPPORT_CONCEALED_KONG_AND_MELDED_KONG
            fan_table[CONCEALED_KONG_AND_MELDED_KONG] = 1;
#else
            fan_table[MELDED_KONG] = 1;
            fan_table[CONCEALED_KONG] = 1;
#endif
            switch (concealed_pung_cnt) {  // 暗刻的个数
            case 1: fan_table[TWO_CONCEALED_PUNGS] = 1; break;
            case 2: fan_table[THREE_CONCEALED_PUNGS] = 1; break;
            default: break;
            }
            break;
        case 2:  // 双暗杠
            fan_table[TWO_CONCEALED_KONGS] = 1;
            switch (concealed_pung_cnt) {  // 暗刻的个数
            case 1: fan_table[THREE_CONCEALED_PUNGS] = 1; break;
            case 2: fan_table[FOUR_CONCEALED_PUNGS] = 1; break;
            default: break;
            }
            break;
        default:
            break;
        }
        break;
    case 3:  // 3个杠
        fan_table[THREE_KONGS] = 1;
        switch (concealed_kong_cnt) {  // 暗刻的个数
        case 0:  // 3明杠
            break;
        case 1:  // 1暗杠2明杠
            if (concealed_pung_cnt > 0) {
                fan_table[TWO_CONCEALED_PUNGS] = 1;
            }
            break;
        case 2:  // 2暗杠1明杠
            if (concealed_pung_cnt == 0) {
                fan_table[TWO_CONCEALED_PUNGS] = 1;
            }
            else {
                fan_table[THREE_CONCEALED_PUNGS] = 1;
            }
            break;
        case 3:  // 3暗杠
            if (concealed_pung_cnt == 0) {
                fan_table[THREE_CONCEALED_PUNGS] = 1;
            }
            else {
                fan_table[FOUR_CONCEALED_PUNGS] = 1;
            }
            break;
        default:
            break;
        }
        break;
    case 4:  // 4个杠
        fan_table[FOUR_KONGS] = 1;
        switch (concealed_kong_cnt) {
        case 2: fan_table[TWO_CONCEALED_PUNGS] = 1; break;
        case 3: fan_table[THREE_CONCEALED_PUNGS] = 1; break;
        case 4: fan_table[FOUR_CONCEALED_PUNGS] = 1; break;
        default: break;
        }
        break;
    default:
        break;
    }

    // 四杠和四暗刻不计碰碰和，其他先加上碰碰和的番
    if (pung_cnt == 4) {
        if (fan_table[FOUR_KONGS] == 0 && fan_table[FOUR_CONCEALED_PUNGS] == 0) {
            fan_table[ALL_PUNGS] = 1;
        }
    }

    // 逐组刻子的番（箭刻、幺九刻）
    for (intptr_t i = 0; i < pung_cnt; ++i) {
        fan_t fan = get_1_pung_fan(pack_get_tile(pung_packs[i]));
        if (fan != FAN_NONE) {
            ++fan_table[fan];
        }
    }
}

// 4组刻子算番
static void calculate_4_pungs(const tile_t (&mid_tiles)[4], fan_table_t &fan_table) {
    fan_t fan;
    // 存在4组刻子的番种时，不再检测其他的了
    if ((fan = get_4_pungs_fan(mid_tiles[0], mid_tiles[1], mid_tiles[2], mid_tiles[3])) != FAN_NONE) {
        fan_table[fan] = 1;
        return;
    }

    // 3组刻子判断
    bool _3_pungs_has_fan = false;
    int free_pack_idx = -1;  // 未使用的1组刻子
    // 012构成3组刻子的番种
    if ((fan = get_3_pungs_fan(mid_tiles[0], mid_tiles[1], mid_tiles[2])) != FAN_NONE) {
        fan_table[fan] = 1;
        free_pack_idx = 3;
        _3_pungs_has_fan = true;
    }
    // 013构成3组刻子的番种
    else if ((fan = get_3_pungs_fan(mid_tiles[0], mid_tiles[1], mid_tiles[3])) != FAN_NONE) {
        fan_table[fan] = 1;
        free_pack_idx = 2;
        _3_pungs_has_fan = true;
    }
    // 023构成3组刻子的番种
    else if ((fan = get_3_pungs_fan(mid_tiles[0], mid_tiles[2], mid_tiles[3])) != FAN_NONE) {
        fan_table[fan] = 1;
        free_pack_idx = 1;
        _3_pungs_has_fan = true;
    }
    // 123构成3组刻子的番种
    else if ((fan = get_3_pungs_fan(mid_tiles[1], mid_tiles[2], mid_tiles[3])) != FAN_NONE) {
        fan_table[fan] = 1;
        free_pack_idx = 0;
        _3_pungs_has_fan = true;
    }

    // 存在3组刻子的番种时，余下的第4组刻子只能组合一次
    if (_3_pungs_has_fan) {
        for (int i = 0; i < 4; ++i) {
            if (i == free_pack_idx) {
                continue;
            }
            // 依次与未使用的这组刻子测试番种
            if ((fan = get_2_pungs_fan_unordered(mid_tiles[i], mid_tiles[free_pack_idx])) != FAN_NONE) {
                ++fan_table[fan];
                break;
            }
        }
        return;
    }

    // 不存在3组刻子的番种时，两两计算番种
    if ((fan = get_2_pungs_fan_unordered(mid_tiles[0], mid_tiles[1])) != FAN_NONE) {
        ++fan_table[fan];
    }
    if ((fan = get_2_pungs_fan_unordered(mid_tiles[0], mid_tiles[2])) != FAN_NONE) {
        ++fan_table[fan];
    }
    if ((fan = get_2_pungs_fan_unordered(mid_tiles[0], mid_tiles[3])) != FAN_NONE) {
        ++fan_table[fan];
    }
    if ((fan = get_2_pungs_fan_unordered(mid_tiles[1], mid_tiles[2])) != FAN_NONE) {
        ++fan_table[fan];
    }
    if ((fan = get_2_pungs_fan_unordered(mid_tiles[1], mid_tiles[3])) != FAN_NONE) {
        ++fan_table[fan];
    }
    if ((fan = get_2_pungs_fan_unordered(mid_tiles[2], mid_tiles[3])) != FAN_NONE) {
        ++fan_table[fan];
    }
}

// 3组刻子算番
static void calculate_3_pungs(const tile_t (&mid_tiles)[3], fan_table_t &fan_table) {
    fan_t fan;

    // 存在3组刻子的番种（三节高 三同刻 三风刻 大三元）时，不再检测其他的
    if ((fan = get_3_pungs_fan(mid_tiles[0], mid_tiles[1], mid_tiles[2])) != FAN_NONE) {
        fan_table[fan] = 1;
        return;
    }

    // 不存在3组刻子的番种时，两两计算番种
    if ((fan = get_2_pungs_fan_unordered(mid_tiles[0], mid_tiles[1])) != FAN_NONE) {
        ++fan_table[fan];
    }
    if ((fan = get_2_pungs_fan_unordered(mid_tiles[0], mid_tiles[2])) != FAN_NONE) {
        ++fan_table[fan];
    }
    if ((fan = get_2_pungs_fan_unordered(mid_tiles[1], mid_tiles[2])) != FAN_NONE) {
        ++fan_table[fan];
    }
}

// 2组刻子算番
static void calculate_2_pungs_unordered(const tile_t (&mid_tiles)[2], fan_table_t &fan_table) {
    fan_t fan = get_2_pungs_fan_unordered(mid_tiles[0], mid_tiles[1]);
    if (fan != FAN_NONE) {
        ++fan_table[fan];
    }
}

// 九莲宝灯
static bool is_nine_gates(const tile_t *tiles) {
    // 对立牌的种类进行打表
    tile_table_t cnt_table;
    map_tiles(tiles, 13, &cnt_table);

    // 1、9各三枚，2~8各一枚
    return (cnt_table[0x11] == 3 && cnt_table[0x19] == 3 && std::all_of(cnt_table + 0x12, cnt_table + 0x19, [](int n) { return n == 1; }))
        || (cnt_table[0x21] == 3 && cnt_table[0x29] == 3 && std::all_of(cnt_table + 0x22, cnt_table + 0x29, [](int n) { return n == 1; }))
        || (cnt_table[0x31] == 3 && cnt_table[0x39] == 3 && std::all_of(cnt_table + 0x32, cnt_table + 0x39, [](int n) { return n == 1; }));
}

// 一色双龙会
static bool is_pure_terminal_chows(const pack_t (&chow_packs)[4], pack_t pair_pack) {
    if (tile_get_rank(pack_get_tile(pair_pack)) != 5) {  // 5作雀头
        return false;
    }

    int _123_cnt = 0, _789_cnt = 0;
    suit_t pair_suit = tile_get_suit(pack_get_tile(pair_pack));
    for (int i = 0; i < 4; ++i) {
        suit_t suit = tile_get_suit(pack_get_tile(chow_packs[i]));
        if (suit != pair_suit) {  // 花色与雀头相同
            return false;
        }
        rank_t rank = tile_get_rank(pack_get_tile(chow_packs[i]));
        switch (rank) {
        case 2: ++_123_cnt; break;
        case 8: ++_789_cnt; break;
        default: return false;
        }
    }
    return (_123_cnt == 2 && _789_cnt == 2);  // 123和789各2组
}

// 三色双龙会
static bool is_three_suited_terminal_chows(const pack_t (&chow_packs)[4], pack_t pair_pack) {
    if (tile_get_rank(pack_get_tile(pair_pack)) != 5) {  // 5作雀头
        return false;
    }

    int _123_suit_table[4] = { 0 };
    int _789_suit_table[4] = { 0 };
    suit_t pair_suit = tile_get_suit(pack_get_tile(pair_pack));
    for (int i = 0; i < 4; ++i) {
        suit_t suit = tile_get_suit(pack_get_tile(chow_packs[i]));
        if (suit == pair_suit) {  // 花色与雀头不相同
            return false;
        }
        rank_t rank = tile_get_rank(pack_get_tile(chow_packs[i]));
        switch (rank) {
        case 2: ++_123_suit_table[suit]; break;
        case 8: ++_789_suit_table[suit]; break;
        default: return false;
        }
    }

    // 与雀头花色不同的两色123和789各一组
    switch (pair_suit) {
    case 1: return (_123_suit_table[2] && _123_suit_table[3] && _789_suit_table[2] && _789_suit_table[3]);
    case 2: return (_123_suit_table[1] && _123_suit_table[3] && _789_suit_table[1] && _789_suit_table[3]);
    case 3: return (_123_suit_table[1] && _123_suit_table[2] && _789_suit_table[1] && _789_suit_table[2]);
    default: return false;
    }
}

// 根据和牌方式调整——涉及番种：不求人、全求人
static void adjust_by_self_drawn(const pack_t (&packs)[5], intptr_t fixed_cnt, bool self_drawn, fan_table_t &fan_table) {
    ptrdiff_t melded_cnt = std::count_if(&packs[0], &packs[fixed_cnt], &is_pack_melded);  // 明副露的组数

    switch (melded_cnt) {
    case 0:  // 0组明的，自摸为不求人，点和为门前清
        fan_table[self_drawn ? FULLY_CONCEALED_HAND : CONCEALED_HAND] = 1;
        break;
    case 4:
        // 4组明的，自摸还是自摸，点和为全求人
        fan_table[self_drawn ? SELF_DRAWN : MELDED_HAND] = 1;
        break;
    default:
        if (self_drawn) {
            fan_table[SELF_DRAWN] = 1;
        }
        break;
    }
}

// 根据雀头调整——涉及番种：平和、小三元、小四喜
static void adjust_by_pair_tile(tile_t pair_tile, intptr_t chow_cnt, fan_table_t &fan_table) {
    if (chow_cnt == 4) {  // 4组都是顺子
        if (is_numbered_suit_quick(pair_tile)) {  // 数牌雀头
            fan_table[ALL_CHOWS] = 1;  // 平和
        }
        return;
    }

    // 在双箭刻的基础上，如果雀头是箭牌，则修正为小三元
    if (fan_table[TWO_DRAGONS_PUNGS]) {
        if (is_dragons(pair_tile)) {
            fan_table[LITTLE_THREE_DRAGONS] = 1;
            fan_table[TWO_DRAGONS_PUNGS] = 0;
        }
        return;
    }
    // 在三风刻的基础上，如果雀头是风牌，则修正为小四喜
    if (fan_table[BIG_THREE_WINDS]) {
        if (is_winds(pair_tile)) {
            fan_table[LITTLE_FOUR_WINDS] = 1;
            fan_table[BIG_THREE_WINDS] = 0;
        }
        return;
    }
}

// 根据花色调整——涉及番种：无字、缺一门、混一色、清一色、五门齐
static void adjust_by_suits(const tile_t *tiles, intptr_t tile_cnt, fan_table_t &fan_table) {
    // 打表标记有哪些花色，用bit操作
    uint8_t suit_flag = 0;
    for (intptr_t i = 0; i < tile_cnt; ++i) {
        suit_flag |= (1 << tile_get_suit(tiles[i]));
    }

    // 1111 0001
    if (!(suit_flag & 0xF1U)) {
        fan_table[NO_HONORS] = 1;  // 无字
    }

    // 1110 0011
    if (!(suit_flag & 0xE3U)) {
        ++fan_table[ONE_VOIDED_SUIT];  // 缺一门（万）
    }
    // 1110 0101
    if (!(suit_flag & 0xE5U)) {
        ++fan_table[ONE_VOIDED_SUIT];  // 缺一门（条）
    }
    // 1110 1001
    if (!(suit_flag & 0xE9U)) {
        ++fan_table[ONE_VOIDED_SUIT];  // 缺一门（饼）
    }

    // 当缺2门时，根据有字和无字，修正为混一色和清一色
    if (fan_table[ONE_VOIDED_SUIT] == 2) {
        fan_table[ONE_VOIDED_SUIT] = 0;
        fan_table[suit_flag & 0xF1U ? HALF_FLUSH : FULL_FLUSH] = 1;
    }

    // 0001 1110
    if (suit_flag == 0x1EU) {  // 三门数牌和字牌都有
        if (std::any_of(tiles, tiles + tile_cnt, &is_winds)
            && std::any_of(tiles, tiles + tile_cnt, &is_dragons)) {
            fan_table[ALL_TYPES] = 1;  // 五门齐
        }
    }
}

// 根据数牌的范围调整——涉及番种：大于五、小于五、全大、全中、全小
static void adjust_by_rank_range(const tile_t *tiles, intptr_t tile_cnt, fan_table_t &fan_table) {
#ifdef STRICT_98_RULE
    if (fan_table[SEVEN_PAIRS]) {
        return;  // 严格98规则的七对不支持叠加这些
    }
#endif

    // 打表标记有哪些数
    uint16_t rank_flag = 0;
    for (intptr_t i = 0; i < tile_cnt; ++i) {
        if (!is_numbered_suit_quick(tiles[i])) {
            return;
        }
        rank_flag |= (1 << tile_get_rank(tiles[i]));
    }

    // 1111 1111 1110 0001
    // 检测是否只包含1234
    if (!(rank_flag & 0xFFE1)) {
        // 包含4为小于五，否则为全小
        fan_table[rank_flag & 0x0010 ? LOWER_FOUR : LOWER_TILES] = 1;
        return;
    }
    // 1111 1100 0011 1111
    // 检测是否只包含6789
    if (!(rank_flag & 0xFC3F)) {
        // 包含6为大于五，否则为全大
        fan_table[rank_flag & 0x0040 ? UPPER_FOUR : UPPER_TILES] = 1;
        return;
    }
    // 1111 1111 1000 1111
    // 检测是否只包含456
    if (!(rank_flag & 0xFF8F)) {
        // 全中
        fan_table[MIDDLE_TILES] = 1;
    }
}

// 根据牌组特征调整——涉及番种：全带幺、全带五、全双刻
static void adjust_by_packs_traits(const pack_t (&packs)[5], fan_table_t &fan_table) {
    // 统计包含数牌19、字牌、5、双数牌的组数
    int terminal_pack = 0;
    int honor_pack = 0;
    int _5_pack = 0;
    int even_pack = 0;
    for (int i = 0; i < 5; ++i) {
        tile_t tile = pack_get_tile(packs[i]);
        if (is_numbered_suit_quick(tile)) {  // 数牌
            rank_t rank = tile_get_rank(tile);
            if (pack_get_type(packs[i]) == PACK_TYPE_CHOW) {  // 顺子
                switch (rank) {
                case 2: case 8: ++terminal_pack; break;  // 数牌19
                case 4: case 5: case 6: ++_5_pack; break;  // 带五
                default: break;
                }
            }
            else {  // 刻子或雀头
                switch (rank) {
                case 1: case 9: ++terminal_pack; break;  // 数牌19
                case 5: ++_5_pack; break;  // 带五
                case 2: case 4: case 6: case 8: ++even_pack; break;  // 双刻
                default: break;
                }
            }
        }
        else {
            ++honor_pack;  // 字牌
        }
    }

    // 5组牌都包含数牌19和字牌，先暂时计为全带幺
    if (terminal_pack + honor_pack == 5) {
        fan_table[OUTSIDE_HAND] = 1;
        return;
    }
    // 全带五
    if (_5_pack == 5) {
        fan_table[ALL_FIVE] = 1;
        return;
    }
    // 全双刻
    if (even_pack == 5) {
        fan_table[ALL_EVEN_PUNGS] = 1;
    }
}

// 根据牌特性调整——涉及番种：断幺、推不倒、绿一色、字一色、清幺九、混幺九
static void adjust_by_tiles_traits(const tile_t *tiles, intptr_t tile_cnt, fan_table_t &fan_table) {
    // 断幺
    if (std::none_of(tiles, tiles + tile_cnt, &is_terminal_or_honor)) {
        fan_table[ALL_SIMPLES] = 1;
    }

    // 推不倒
    if (std::all_of(tiles, tiles + tile_cnt, &is_reversible)) {
        fan_table[REVERSIBLE_TILES] = 1;
    }

#ifdef STRICT_98_RULE
    if (fan_table[SEVEN_PAIRS]) {
        return;  // 严格98规则的七对不支持绿一色、字一色、清幺九、混幺九
    }
#endif

    // 绿一色
    if (std::all_of(tiles, tiles + tile_cnt, &is_green)) {
        fan_table[ALL_GREEN] = 1;
    }

    // 如果断幺了就没必要检测字一色、清幺九、混幺九了
    if (fan_table[ALL_SIMPLES] != 0) {
        return;
    }

    // 字一色
    if (std::all_of(tiles, tiles + tile_cnt, &is_honor)) {
        fan_table[ALL_HONORS] = 1;
        return;
    }
    // 清幺九
    if (std::all_of(tiles, tiles + tile_cnt, &is_terminal)) {
        fan_table[ALL_TERMINALS] = 1;
        return;
    }
    // 混幺九
    if (std::all_of(tiles, tiles + tile_cnt, &is_terminal_or_honor)) {
        fan_table[ALL_TERMINALS_AND_HONORS] = 1;
    }
}

// 四归一调整
static void adjust_by_tiles_hog(const tile_t *tiles, intptr_t tile_cnt, fan_table_t &fan_table) {
    intptr_t kong_cnt = tile_cnt - 14;  // 标准和牌14张，多出几张就说明有几个杠
    tile_table_t cnt_table;
    map_tiles(tiles, tile_cnt, &cnt_table);
    // 有多少种已经用去4张的牌减去杠的数量，即为四归一的数量
    intptr_t _4_cnt = std::count(std::begin(cnt_table), std::end(cnt_table), 4);
    fan_table[TILE_HOG] = static_cast<uint8_t>(_4_cnt - kong_cnt);
}

// 根据听牌方式调整——涉及番种：边张、嵌张、单钓将
static void adjust_by_waiting_form(const pack_t *concealed_packs, intptr_t pack_cnt, const tile_t *standing_tiles, intptr_t standing_cnt,
    tile_t win_tile, fan_table_t &fan_table) {
    // 全求人和四杠不计单钓将，也不可能有边张、嵌张
    if (fan_table[MELDED_HAND] || fan_table[FOUR_KONGS]) {
        return;
    }

    useful_table_t waiting_table;  // 听牌标记表
    if (!is_basic_form_wait(standing_tiles, standing_cnt, &waiting_table)) {
        return;
    }

    if (pack_cnt == 5) {  // 门清状态
        // 判断是否为七对听牌
        useful_table_t temp_table;
        if (is_seven_pairs_wait(standing_tiles, standing_cnt, &temp_table)) {
            // 合并听牌标记表
            std::transform(std::begin(temp_table), std::end(temp_table), std::begin(waiting_table),
                std::begin(waiting_table), [](bool w, bool t) { return w || t; });
        }
    }

    // 统计听牌张数，听牌数大于1张，不计边张、嵌张、单钓将
    if (1 != std::count(std::begin(waiting_table), std::end(waiting_table), true)) {
        return;
    }

    // 听1张的情况，看和牌张处于什么位置
    // 边张0x01 嵌张0x02 单钓将0x04
    uint8_t pos_flag = 0;

    for (intptr_t i = 0; i < pack_cnt; ++i) {
        switch (pack_get_type(concealed_packs[i])) {
        case PACK_TYPE_CHOW: {
            tile_t mid_tile = pack_get_tile(concealed_packs[i]);
            if (mid_tile == win_tile) {
                pos_flag |= 0x02U;  // 嵌张
            }
            else if (mid_tile + 1 == win_tile || mid_tile - 1 == win_tile) {
                pos_flag |= 0x01U;  // 边张
            }
            break;
        }
        case PACK_TYPE_PAIR: {
            tile_t mid_tile = pack_get_tile(concealed_packs[i]);
            if (mid_tile == win_tile) {
                pos_flag |= 0x04U;  // 单钓将
            }
            break;
        }
        default:
            break;
        }
    }

    // 当多种可能存在时，只能计其中一种
    if (pos_flag & 0x01U) {
        fan_table[EDGE_WAIT] = 1;
    }
    else if (pos_flag & 0x02U) {
        fan_table[CLOSED_WAIT] = 1;
    }
    else if (pos_flag & 0x04U) {
        fan_table[SINGLE_WAIT] = 1;
    }
}

// 根据风调整——涉及番种：圈风刻、门风刻
static void adjust_by_winds(tile_t tile, wind_t prevalent_wind, wind_t seat_wind, fan_table_t &fan_table) {
    rank_t delta = tile - TILE_E;
    if (delta == static_cast<int>(prevalent_wind) - static_cast<int>(wind_t::EAST)) {
        fan_table[PREVALENT_WIND] = 1;
    }
    if (delta == static_cast<int>(seat_wind) - static_cast<int>(wind_t::EAST)) {
        fan_table[SEAT_WIND] = 1;
    }
}

// 统一调整一些不计的
static void adjust_fan_table(fan_table_t &fan_table, bool prevalent_eq_seat) {
    // 大四喜不计三风刻、碰碰和、圈风刻、门风刻、幺九刻
    if (fan_table[BIG_FOUR_WINDS]) {
        fan_table[BIG_THREE_WINDS] = 0;
        fan_table[ALL_PUNGS] = 0;
        fan_table[PREVALENT_WIND] = 0;
        fan_table[SEAT_WIND] = 0;
        fan_table[PUNG_OF_TERMINALS_OR_HONORS] = 0;
    }
    // 大三元不计双箭刻、箭刻（严格98规则不计缺一门）
    if (fan_table[BIG_THREE_DRAGONS]) {
        fan_table[TWO_DRAGONS_PUNGS] = 0;
        fan_table[DRAGON_PUNG] = 0;
#ifdef STRICT_98_RULE
        fan_table[ONE_VOIDED_SUIT] = 0;
#endif
    }
    // 绿一色不计混一色、缺一门
    if (fan_table[ALL_GREEN]) {
        fan_table[HALF_FLUSH] = 0;
        fan_table[ONE_VOIDED_SUIT] = 0;
    }
    // 九莲宝灯不计清一色、门前清、缺一门、无字，减计1个幺九刻，把不求人修正为自摸
    if (fan_table[NINE_GATES]) {
        fan_table[FULL_FLUSH] = 0;
        fan_table[CONCEALED_HAND] = 0;
        --fan_table[PUNG_OF_TERMINALS_OR_HONORS];
        fan_table[ONE_VOIDED_SUIT] = 0;
        fan_table[NO_HONORS] = 0;
        if (fan_table[FULLY_CONCEALED_HAND]) {
            fan_table[FULLY_CONCEALED_HAND] = 0;
            fan_table[SELF_DRAWN] = 1;
        }
    }
    // 四杠不计单钓将
    if (fan_table[FOUR_KONGS]) {
        fan_table[SINGLE_WAIT] = 0;
    }
    // 连七对不计七对、清一色、门前清、缺一门、无字
    if (fan_table[SEVEN_SHIFTED_PAIRS]) {
        fan_table[SEVEN_PAIRS] = 0;
        fan_table[FULL_FLUSH] = 0;
        fan_table[CONCEALED_HAND] = 0;
        fan_table[ONE_VOIDED_SUIT] = 0;
        fan_table[NO_HONORS] = 0;
    }
    // 十三幺不计五门齐、门前清、单钓将
    if (fan_table[THIRTEEN_ORPHANS]) {
        fan_table[ALL_TYPES] = 0;
        fan_table[CONCEALED_HAND] = 0;
        fan_table[SINGLE_WAIT] = 0;
    }

    // 清幺九不计混幺九、碰碰胡、全带幺、幺九刻、无字（严格98规则不计双同刻、不计三同刻）
    if (fan_table[ALL_TERMINALS]) {
        fan_table[ALL_TERMINALS_AND_HONORS] = 0;
        fan_table[ALL_PUNGS] = 0;
        fan_table[OUTSIDE_HAND] = 0;
        fan_table[PUNG_OF_TERMINALS_OR_HONORS] = 0;
        fan_table[NO_HONORS] = 0;
        fan_table[DOUBLE_PUNG] = 0;  // 通行计法不计双同刻
#ifdef STRICT_98_RULE
        fan_table[TRIPLE_PUNG] = 0;
        fan_table[DOUBLE_PUNG] = 0;
#endif
    }

    // 小四喜不计三风刻
    if (fan_table[LITTLE_FOUR_WINDS]) {
        fan_table[BIG_THREE_WINDS] = 0;
        // 小四喜的第四组牌如果是19的刻子，则是混幺九；如果是箭刻则是字一色；这两种都是不计幺九刻的
        // 如果是顺子或者2-8的刻子，则不存在多余的幺九刻
        // 所以这里将幺九刻置为0
        fan_table[PUNG_OF_TERMINALS_OR_HONORS] = 0;
    }

    // 小三元不计双箭刻、箭刻（严格98规则不计缺一门）
    if (fan_table[LITTLE_THREE_DRAGONS]) {
        fan_table[TWO_DRAGONS_PUNGS] = 0;
        fan_table[DRAGON_PUNG] = 0;
#ifdef STRICT_98_RULE
        fan_table[ONE_VOIDED_SUIT] = 0;
#endif
    }

    // 字一色不计混幺九、碰碰胡、全带幺、幺九刻、缺一门
    if (fan_table[ALL_HONORS]) {
        fan_table[ALL_TERMINALS_AND_HONORS] = 0;
        fan_table[ALL_PUNGS] = 0;
        fan_table[OUTSIDE_HAND] = 0;
        fan_table[PUNG_OF_TERMINALS_OR_HONORS] = 0;
        fan_table[ONE_VOIDED_SUIT] = 0;
    }
    // 四暗刻不计碰碰和、门前清，把不求人修正为自摸
    if (fan_table[FOUR_CONCEALED_PUNGS]) {
        fan_table[ALL_PUNGS] = 0;
        fan_table[CONCEALED_HAND] = 0;
        if (fan_table[FULLY_CONCEALED_HAND]) {
            fan_table[FULLY_CONCEALED_HAND] = 0;
            fan_table[SELF_DRAWN] = 1;
        }
    }
    // 一色双龙会不计七对、清一色、平和、一般高、老少副、缺一门、无字
    if (fan_table[PURE_TERMINAL_CHOWS]) {
        fan_table[SEVEN_PAIRS] = 0;
        fan_table[FULL_FLUSH] = 0;
        fan_table[ALL_CHOWS] = 0;
        fan_table[PURE_DOUBLE_CHOW] = 0;
        fan_table[TWO_TERMINAL_CHOWS] = 0;
        fan_table[ONE_VOIDED_SUIT] = 0;
        fan_table[NO_HONORS] = 0;
    }

    // 一色四同顺不计一色三同顺、一般高、四归一（严格98规则不计缺一门）
    if (fan_table[QUADRUPLE_CHOW]) {
        fan_table[PURE_SHIFTED_PUNGS] = 0;
        fan_table[TILE_HOG] = 0;
        fan_table[PURE_DOUBLE_CHOW] = 0;
#ifdef STRICT_98_RULE
        fan_table[ONE_VOIDED_SUIT] = 0;
#endif
    }
    // 一色四节高不计一色三节高、碰碰和（严格98规则不计缺一门）
    if (fan_table[FOUR_PURE_SHIFTED_PUNGS]) {
        fan_table[PURE_TRIPLE_CHOW] = 0;
        fan_table[ALL_PUNGS] = 0;
#ifdef STRICT_98_RULE
        fan_table[ONE_VOIDED_SUIT] = 0;
#endif
    }

    // 一色四步高不计一色三步高、老少副、连六（严格98规则不计缺一门）
    if (fan_table[FOUR_PURE_SHIFTED_CHOWS]) {
        fan_table[PURE_SHIFTED_CHOWS] = 0;
        fan_table[TWO_TERMINAL_CHOWS] = 0;
        fan_table[SHORT_STRAIGHT] = 0;
#ifdef STRICT_98_RULE
        fan_table[ONE_VOIDED_SUIT] = 0;
#endif
    }

    // 混幺九不计碰碰和、全带幺、幺九刻
    if (fan_table[ALL_TERMINALS_AND_HONORS]) {
        fan_table[ALL_PUNGS] = 0;
        fan_table[OUTSIDE_HAND] = 0;
        fan_table[PUNG_OF_TERMINALS_OR_HONORS] = 0;
    }

    // 七对不计门前清、单钓将
    if (fan_table[SEVEN_PAIRS]) {
        fan_table[CONCEALED_HAND] = 0;
        fan_table[SINGLE_WAIT] = 0;
    }
    // 七星不靠不计五门齐、门前清
    if (fan_table[GREATER_HONORS_AND_KNITTED_TILES]) {
        fan_table[ALL_TYPES] = 0;
        fan_table[CONCEALED_HAND] = 0;
    }
    // 全双刻不计碰碰胡、断幺、无字
    if (fan_table[ALL_EVEN_PUNGS]) {
        fan_table[ALL_PUNGS] = 0;
        fan_table[ALL_SIMPLES] = 0;
        fan_table[NO_HONORS] = 0;
    }
    // 清一色不计缺一门、无字
    if (fan_table[FULL_FLUSH]) {
        fan_table[ONE_VOIDED_SUIT] = 0;
        fan_table[NO_HONORS] = 0;
    }
    // 一色三同顺不计一色三节高、一般高
    if (fan_table[PURE_TRIPLE_CHOW]) {
        fan_table[PURE_SHIFTED_PUNGS] = 0;
        fan_table[PURE_DOUBLE_CHOW] = 0;
    }
    // 一色三节高不计一色三同顺
    if (fan_table[PURE_SHIFTED_PUNGS]) {
        fan_table[PURE_TRIPLE_CHOW] = 0;
    }
    // 全大不计大于五、无字
    if (fan_table[UPPER_TILES]) {
        fan_table[UPPER_FOUR] = 0;
        fan_table[NO_HONORS] = 0;
    }
    // 全中不计断幺
    if (fan_table[MIDDLE_TILES]) {
        fan_table[ALL_SIMPLES] = 0;
        fan_table[NO_HONORS] = 0;
    }
    // 全小不计小于五、无字
    if (fan_table[LOWER_TILES]) {
        fan_table[LOWER_FOUR] = 0;
        fan_table[NO_HONORS] = 0;
    }

    // 三色双龙会不计平和、无字、喜相逢、老少副
    if (fan_table[THREE_SUITED_TERMINAL_CHOWS]) {
        fan_table[ALL_CHOWS] = 0;
        fan_table[NO_HONORS] = 0;
        fan_table[MIXED_DOUBLE_CHOW] = 0;
        fan_table[TWO_TERMINAL_CHOWS] = 0;
    }
    // 全带五不计断幺、无字
    if (fan_table[ALL_FIVE]) {
        fan_table[ALL_SIMPLES] = 0;
        fan_table[NO_HONORS] = 0;
    }

    // 七星不靠不计五门齐、门前清
    if (fan_table[LESSER_HONORS_AND_KNITTED_TILES]) {
        fan_table[ALL_TYPES] = 0;
        fan_table[CONCEALED_HAND] = 0;
    }
    // 大于五不计无字
    if (fan_table[UPPER_FOUR]) {
        fan_table[NO_HONORS] = 0;
    }
    // 小于五不计无字
    if (fan_table[LOWER_FOUR]) {
        fan_table[NO_HONORS] = 0;
    }
    // 三风刻内部不再计幺九刻（严格98规则不计缺一门）
    if (fan_table[BIG_THREE_WINDS]) {
        // 如果不是字一色或混幺九，则要减去3个幺九刻
        if (!fan_table[ALL_HONORS] && !fan_table[ALL_TERMINALS_AND_HONORS]) {
            assert(fan_table[PUNG_OF_TERMINALS_OR_HONORS] >= 3);
            fan_table[PUNG_OF_TERMINALS_OR_HONORS] -= 3;
        }
#ifdef STRICT_98_RULE
        fan_table[ONE_VOIDED_SUIT] = 0;
#endif
    }

    // 推不倒不计缺一门
    if (fan_table[REVERSIBLE_TILES]) {
        fan_table[ONE_VOIDED_SUIT] = 0;
    }
    // 妙手回春不计自摸
    if (fan_table[LAST_TILE_DRAW]) {
        fan_table[SELF_DRAWN] = 0;
    }
    // 杠上开花不计自摸
    if (fan_table[OUT_WITH_REPLACEMENT_TILE]) {
        fan_table[SELF_DRAWN] = 0;
    }
    // 抢杠和不计和绝张
    if (fan_table[ROBBING_THE_KONG]) {
        fan_table[LAST_TILE] = 0;
    }
    // 双暗杠不计暗杠
    if (fan_table[TWO_CONCEALED_KONGS]) {
        fan_table[CONCEALED_KONG] = 0;
    }

    // 混一色不计缺一门
    if (fan_table[HALF_FLUSH]) {
        fan_table[ONE_VOIDED_SUIT] = 0;
    }
    // 全求人不计单钓将
    if (fan_table[MELDED_HAND]) {
        fan_table[SINGLE_WAIT] = 0;
    }
    // 双箭刻不计箭刻
    if (fan_table[TWO_DRAGONS_PUNGS]) {
        fan_table[DRAGON_PUNG] = 0;
    }

    // 不求人不计自摸
    if (fan_table[FULLY_CONCEALED_HAND]) {
        fan_table[SELF_DRAWN] = 0;
    }
    // 双明杠不计明杠
    if (fan_table[TWO_MELDED_KONGS]) {
        fan_table[MELDED_KONG] = 0;
    }

    // 圈风刻自己不计幺九刻
    if (fan_table[PREVALENT_WIND]) {
        // 如果不是三风刻、小四喜、字一色、混幺九，则要减去1个幺九刻
        if (!fan_table[BIG_THREE_WINDS] && !fan_table[LITTLE_FOUR_WINDS]
            && !fan_table[ALL_HONORS] && !fan_table[ALL_TERMINALS_AND_HONORS]) {
            assert(fan_table[PUNG_OF_TERMINALS_OR_HONORS] > 0);
            --fan_table[PUNG_OF_TERMINALS_OR_HONORS];
        }
    }
    // 门风刻自己不计幺九刻
    if (fan_table[SEAT_WIND]) {
        // 如果圈风与门风不相同，并且不是三风刻、小四喜、字一色、混幺九，则要减去1个幺九刻
        if (!prevalent_eq_seat && !fan_table[BIG_THREE_WINDS] && !fan_table[LITTLE_FOUR_WINDS]
            && !fan_table[ALL_HONORS] && !fan_table[ALL_TERMINALS_AND_HONORS]) {
            assert(fan_table[PUNG_OF_TERMINALS_OR_HONORS] > 0);
            --fan_table[PUNG_OF_TERMINALS_OR_HONORS];
        }
    }
    // 平和不计无字
    if (fan_table[ALL_CHOWS]) {
        fan_table[NO_HONORS] = 0;
    }
    // 断幺不计无字
    if (fan_table[ALL_SIMPLES]) {
        fan_table[NO_HONORS] = 0;
    }
}

// 根据和牌标记调整——涉及番种：和绝张、妙手回春、海底捞月、自摸
static void adjust_by_win_flag(win_flag_t win_flag, fan_table_t &fan_table) {
    if (win_flag & WIN_FLAG_4TH_TILE) {
        fan_table[LAST_TILE] = 1;
    }
    if (win_flag & WIN_FLAG_WALL_LAST) {
        fan_table[win_flag & WIN_FLAG_SELF_DRAWN ? LAST_TILE_DRAW : LAST_TILE_CLAIM] = 1;
    }
    if (win_flag & WIN_FLAG_ABOUT_KONG) {
        fan_table[win_flag & WIN_FLAG_SELF_DRAWN ? OUT_WITH_REPLACEMENT_TILE : ROBBING_THE_KONG] = 1;
    }
    if (win_flag & WIN_FLAG_SELF_DRAWN) {
        fan_table[SELF_DRAWN] = 1;
    }
}

// 基本和型算番
static void calculate_basic_form_fan(const pack_t (&packs)[5], const calculate_param_t *calculate_param, fan_table_t &fan_table) {
    pack_t pair_pack = 0;
    pack_t chow_packs[4];
    pack_t pung_packs[4];
    intptr_t chow_cnt = 0;
    intptr_t pung_cnt = 0;
    for (int i = 0; i < 5; ++i) {
        switch (pack_get_type(packs[i])) {
        case PACK_TYPE_CHOW: chow_packs[chow_cnt++] = packs[i]; break;
        case PACK_TYPE_PUNG:
        case PACK_TYPE_KONG: pung_packs[pung_cnt++] = packs[i]; break;
        case PACK_TYPE_PAIR: pair_pack = packs[i]; break;
        default: assert(0); return;
        }
    }

    if (pair_pack == 0 || chow_cnt + pung_cnt != 4) {
        return;
    }

    tile_t win_tile = calculate_param->win_tile;
    win_flag_t win_flag = calculate_param->win_flag;

    // 根据和牌标记调整——涉及番种：和绝张、妙手回春、海底捞月、自摸
    adjust_by_win_flag(win_flag, fan_table);

    // 点和的牌张，如果不能解释为顺子中的一张，那么将其解释为刻子，并标记这个刻子为明刻
    if ((win_flag & WIN_FLAG_SELF_DRAWN) == 0) {
        // 和牌不能解释为顺子中的一张
        if (std::none_of(chow_packs, chow_packs + chow_cnt, [win_tile](pack_t chow_pack) {
            tile_t tile = pack_get_tile(chow_pack);
            return !is_pack_melded(chow_pack)
                && (tile - 1 == win_tile || tile == win_tile || tile + 1 == win_tile);
        })) {
            for (intptr_t i = 0; i < pung_cnt; ++i) {
                if (pack_get_tile(pung_packs[i]) == win_tile && !is_pack_melded(pung_packs[i])) {
                    pung_packs[i] |= (1 << 12);  // 标记为明副露
                }
            }
        }
    }

    if (pung_cnt > 0) { // 有刻子
        calculate_kongs(pung_packs, pung_cnt, fan_table);
    }

    switch (chow_cnt) {
    case 4: {  // 4组顺子
        // 检测三色/一色双龙会
        if (is_three_suited_terminal_chows(chow_packs, pair_pack)) {
            fan_table[THREE_SUITED_TERMINAL_CHOWS] = 1;
            break;
        }
        if (is_pure_terminal_chows(chow_packs, pair_pack)) {
            fan_table[PURE_TERMINAL_CHOWS] = 1;
            break;
        }

        tile_t mid_tiles[4];
        mid_tiles[0] = pack_get_tile(chow_packs[0]);
        mid_tiles[1] = pack_get_tile(chow_packs[1]);
        mid_tiles[2] = pack_get_tile(chow_packs[2]);
        mid_tiles[3] = pack_get_tile(chow_packs[3]);
        std::sort(std::begin(mid_tiles), std::end(mid_tiles));

        calculate_4_chows(mid_tiles, fan_table);
        break;
    }
    case 3: {  // 3组顺子+1组刻子
        tile_t mid_tiles[3];
        mid_tiles[0] = pack_get_tile(chow_packs[0]);
        mid_tiles[1] = pack_get_tile(chow_packs[1]);
        mid_tiles[2] = pack_get_tile(chow_packs[2]);
        std::sort(std::begin(mid_tiles), std::end(mid_tiles));

        calculate_3_chows(mid_tiles, fan_table);
        break;
    }
    case 2: {  // 2组顺子+2组刻子
        tile_t mid_tiles_chow[2];
        mid_tiles_chow[0] = pack_get_tile(chow_packs[0]);
        mid_tiles_chow[1] = pack_get_tile(chow_packs[1]);

        tile_t mid_tiles_pung[2];
        mid_tiles_pung[0] = pack_get_tile(pung_packs[0]);
        mid_tiles_pung[1] = pack_get_tile(pung_packs[1]);

        calculate_2_chows_unordered(mid_tiles_chow, fan_table);
        calculate_2_pungs_unordered(mid_tiles_pung, fan_table);
        break;
    }
    case 1: {  // 1组顺子+3组刻子
        tile_t mid_tiles[3];
        mid_tiles[0] = pack_get_tile(pung_packs[0]);
        mid_tiles[1] = pack_get_tile(pung_packs[1]);
        mid_tiles[2] = pack_get_tile(pung_packs[2]);
        std::sort(std::begin(mid_tiles), std::end(mid_tiles));

        calculate_3_pungs(mid_tiles, fan_table);
        break;
    }
    case 0: {  // 4组刻子
        tile_t mid_tiles[4];
        mid_tiles[0] = pack_get_tile(pung_packs[0]);
        mid_tiles[1] = pack_get_tile(pung_packs[1]);
        mid_tiles[2] = pack_get_tile(pung_packs[2]);
        mid_tiles[3] = pack_get_tile(pung_packs[3]);
        std::sort(std::begin(mid_tiles), std::end(mid_tiles));

        calculate_4_pungs(mid_tiles, fan_table);
        break;
    }
    default:
        break;
    }

    intptr_t fixed_cnt = calculate_param->hand_tiles.pack_count;
    const tile_t *standing_tiles = calculate_param->hand_tiles.standing_tiles;
    intptr_t standing_cnt = calculate_param->hand_tiles.tile_count;
    wind_t prevalent_wind = calculate_param->prevalent_wind;
    wind_t seat_wind = calculate_param->seat_wind;

    // 九莲宝灯
    if (standing_cnt == 13) {
        if (is_nine_gates(standing_tiles)) {
            fan_table[NINE_GATES] = 1;
        }
    }

    // 根据和牌方式调整——涉及番种：不求人、全求人
    adjust_by_self_drawn(packs, fixed_cnt, (win_flag & WIN_FLAG_SELF_DRAWN) != 0, fan_table);
    // 根据雀头调整——涉及番种：平和、小三元、小四喜
    adjust_by_pair_tile(pack_get_tile(pair_pack), chow_cnt, fan_table);
    // 根据牌组特征调整——涉及番种：全带幺、全带五、全双刻
    adjust_by_packs_traits(packs, fan_table);

    tile_t tiles[18];
    memcpy(tiles, standing_tiles, standing_cnt * sizeof(tile_t));
    intptr_t tile_cnt = packs_to_tiles(packs, fixed_cnt, &tiles[standing_cnt], 18 - standing_cnt);
    tile_cnt += standing_cnt;
    tiles[tile_cnt++] = win_tile;

    // 根据花色调整——涉及番种：无字、缺一门、混一色、清一色、五门齐
    adjust_by_suits(tiles, tile_cnt, fan_table);
    // 根据牌特性调整——涉及番种：断幺、推不倒、绿一色、字一色、清幺九、混幺九
    adjust_by_tiles_traits(tiles, tile_cnt, fan_table);
    // 根据数牌的范围调整——涉及番种：大于五、小于五、全大、全中、全小
    adjust_by_rank_range(tiles, tile_cnt, fan_table);
    // 四归一调整
    adjust_by_tiles_hog(tiles, tile_cnt, fan_table);
    // 根据听牌方式调整——涉及番种：边张、嵌张、单钓将
    adjust_by_waiting_form(packs + fixed_cnt, 5 - fixed_cnt, standing_tiles, standing_cnt, win_tile, fan_table);

    // 根据风调整——涉及番种：圈风刻、门风刻
    for (intptr_t i = 0; i < pung_cnt; ++i) {
        adjust_by_winds(pack_get_tile(pung_packs[i]), prevalent_wind, seat_wind, fan_table);
    }

    // 统一调整一些不计的
    adjust_fan_table(fan_table, prevalent_wind == seat_wind);

    // 如果什么番都没有，则计为无番和
    if (std::all_of(std::begin(fan_table), std::end(fan_table), [](uint8_t p) { return p == 0; })) {
        fan_table[CHICKEN_HAND] = 1;
    }
}

// “组合龙+面子+雀头”和型算番
static bool calculate_knitted_straight_fan(const calculate_param_t *calculate_param, fan_table_t &fan_table) {
    const hand_tiles_t *hand_tiles = &calculate_param->hand_tiles;
    tile_t win_tile = calculate_param->win_tile;
    win_flag_t win_flag = calculate_param->win_flag;
    wind_t prevalent_wind = calculate_param->prevalent_wind;
    wind_t seat_wind = calculate_param->seat_wind;

    intptr_t fixed_cnt = hand_tiles->pack_count;
    if (fixed_cnt > 1) {
        return false;
    }

    const pack_t *fixed_packs = hand_tiles->fixed_packs;
    intptr_t standing_cnt = hand_tiles->tile_count;

    // 对立牌和和牌的种类进行打表
    tile_table_t cnt_table;
    map_tiles(hand_tiles->standing_tiles, standing_cnt, &cnt_table);
    ++cnt_table[win_tile];

    // 匹配组合龙
    const tile_t (*matched_seq)[9] = std::find_if(&standard_knitted_straight[0], &standard_knitted_straight[6],
        [&cnt_table](const tile_t (&seq)[9]) {
        return std::all_of(std::begin(seq), std::end(seq), [&cnt_table](tile_t t) { return cnt_table[t] > 0; });
    });

    if (matched_seq == &standard_knitted_straight[6]) {
        return false;
    }

    // 剔除组合龙
    std::for_each(std::begin(*matched_seq), std::end(*matched_seq), [&cnt_table](tile_t t) { --cnt_table[t]; });

    // 按基本和型划分
    division_result_t result;
    result.count = 0;
    division_t work_division;
    memset(&work_division, 0, sizeof(work_division));

    // 此处逻辑为：将组合龙9张牌当作是已经完成的3组面子，空出0 1 2下标处的3组
    // 如果第4组是副露的，将其放在下标3处
    // 然后按基本和型从从fixed_cnt + 3开始递归
    // 划分后的结果，下标3处为第四组面子，下标4处为雀头
    if (fixed_cnt == 1) {
        work_division.packs[3] = fixed_packs[0];
    }
    divide_recursively(cnt_table, fixed_cnt + 3, 0, &work_division, &result);
    if (result.count != 1) {
        return false;
    }

    const pack_t *packs = result.divisions[0].packs;  // packs的0 1 2下标都是没用的

    // 标记番
    fan_table[KNITTED_STRAIGHT] = 1;  // 组合龙
    if (pack_get_type(packs[3]) == PACK_TYPE_CHOW) {  // 第4组是顺子
        if (is_numbered_suit_quick(pack_get_tile(packs[4]))) {
            fan_table[ALL_CHOWS] = 1;  // 雀头是数牌时，为平和
        }
    }
    else {
        calculate_kongs(&packs[3], 1, fan_table);

        // 根据风调整——涉及番种：圈风刻、门风刻
        adjust_by_winds(pack_get_tile(packs[3]), prevalent_wind, seat_wind, fan_table);
    }

    adjust_by_win_flag(win_flag, fan_table);
    // 门前清（暗杠不影响）
    if (fixed_cnt == 0 || (pack_get_type(packs[3]) == PACK_TYPE_KONG && !is_pack_melded(packs[3]))) {
        if (win_flag & WIN_FLAG_SELF_DRAWN) {
            fan_table[FULLY_CONCEALED_HAND] = 1;
        }
        else {
            fan_table[CONCEALED_HAND] = 1;
        }
    }

    // 还原牌
    tile_t tiles[15];  // 第四组可能为杠，所以最多为15张
    memcpy(tiles, matched_seq, sizeof(*matched_seq));  // 组合龙的部分
    intptr_t tile_cnt = packs_to_tiles(&packs[3], 2, tiles + 9, 6);  // 一组面子+一对雀头 最多6张牌
    tile_cnt += 9;

    // 根据花色调整——涉及番种：无字、缺一门、混一色、清一色、五门齐
    adjust_by_suits(tiles, tile_cnt, fan_table);
    // 牌组以及牌特征就不需要调整了，有组合龙的存在绝对不可能存在全带幺、全带五、全双刻，断幺、推不倒、绿一色、字一色、清幺九、混幺九
    // 四归一调整
    adjust_by_tiles_hog(tiles, tile_cnt, fan_table);

    // 和牌张是组合龙范围的牌，不计边张、嵌张、单钓将
    if (std::none_of(std::begin(*matched_seq), std::end(*matched_seq), [win_tile](tile_t t) { return t == win_tile; })) {
        if (fixed_cnt == 0) {  // 门清的牌有可能存在边张、嵌张、单钓将
            // 将除去组合龙的部分恢复成牌
            --cnt_table[win_tile];
            tile_t temp[4];
            intptr_t cnt = table_to_tiles(cnt_table, temp, 4);

            // 根据听牌方式调整——涉及番种：边张、嵌张、单钓将
            adjust_by_waiting_form(packs + 3, 2, temp, cnt, win_tile, fan_table);
        }
        else {
            // 非门清状态如果听牌不在组合龙范围内，必然是单钓将
            fan_table[SINGLE_WAIT] = 1;
        }
    }

    // 统一调整一些不计的
    adjust_fan_table(fan_table, prevalent_wind == seat_wind);
    return true;
}

// 十三幺
static forceinline bool is_thirteen_orphans(const tile_t (&tiles)[14]) {
    return std::all_of(std::begin(tiles), std::end(tiles), &is_terminal_or_honor)
        && std::includes(std::begin(tiles), std::end(tiles),
        std::begin(standard_thirteen_orphans), std::end(standard_thirteen_orphans));
}

// 全不靠/七星不靠算番
static bool calculate_honors_and_knitted_tiles(const tile_t (&standing_tiles)[14], fan_table_t &fan_table) {
    const tile_t *honor_begin = std::find_if(std::begin(standing_tiles), std::end(standing_tiles), &is_honor);
    int numbered_cnt = static_cast<int>(honor_begin - standing_tiles);
    // 数牌张数大于9或者小于7必然不可能是全不靠
    if (numbered_cnt > 9 || numbered_cnt < 7) {
        return false;
    }

    // 匹配组合龙
    if (std::none_of(&standard_knitted_straight[0], &standard_knitted_straight[6],
        [&standing_tiles, honor_begin](const tile_t (&seq)[9]) {
        return std::includes(std::begin(seq), std::end(seq), std::begin(standing_tiles), honor_begin);
    })) {
        return false;
    }

    if (numbered_cnt == 7 && std::equal(std::begin(standard_thirteen_orphans) + 6, std::end(standard_thirteen_orphans), standing_tiles + 7)) {
        // 七种字牌齐，为七星不靠
        fan_table[GREATER_HONORS_AND_KNITTED_TILES] = 1;
        return true;
    }
    else if (std::includes(std::begin(standard_thirteen_orphans) + 6, std::end(standard_thirteen_orphans), honor_begin, std::end(standing_tiles))) {
        // 全不靠
        fan_table[LESSER_HONORS_AND_KNITTED_TILES] = 1;
        if (numbered_cnt == 9) {  // 有9张数牌，为带组合龙的全不靠
            fan_table[KNITTED_STRAIGHT] = 1;
        }
        return true;
    }

    return false;
}

// 特殊和型算番
static bool calculate_special_form_fan(const tile_t (&standing_tiles)[14], win_flag_t win_flag, fan_table_t &fan_table) {
    // 七对
    if (standing_tiles[0] == standing_tiles[1]
        && standing_tiles[2] == standing_tiles[3]
        && standing_tiles[4] == standing_tiles[5]
        && standing_tiles[6] == standing_tiles[7]
        && standing_tiles[8] == standing_tiles[9]
        && standing_tiles[10] == standing_tiles[11]
        && standing_tiles[12] == standing_tiles[13]) {

        if (is_numbered_suit_quick(standing_tiles[0])
            && standing_tiles[0] + 1 == standing_tiles[2]
            && standing_tiles[2] + 1 == standing_tiles[4]
            && standing_tiles[4] + 1 == standing_tiles[6]
            && standing_tiles[6] + 1 == standing_tiles[8]
            && standing_tiles[8] + 1 == standing_tiles[10]
            && standing_tiles[10] + 1 == standing_tiles[12]) {
            // 连七对
            fan_table[SEVEN_SHIFTED_PAIRS] = 1;
            adjust_by_tiles_traits(standing_tiles, 14, fan_table);
        }
        else {
            // 普通七对
            fan_table[SEVEN_PAIRS] = 1;

            // 根据花色调整——涉及番种：无字、缺一门、混一色、清一色、五门齐
            adjust_by_suits(standing_tiles, 14, fan_table);
            // 根据牌特性调整——涉及番种：断幺、推不倒、绿一色、字一色、清幺九、混幺九
            adjust_by_tiles_traits(standing_tiles, 14, fan_table);
            // 根据数牌的范围调整——涉及番种：大于五、小于五、全大、全中、全小
            adjust_by_rank_range(standing_tiles, 14, fan_table);
            // 四归一调整
            adjust_by_tiles_hog(standing_tiles, 14, fan_table);
        }
    }
    // 十三幺
    else if (is_thirteen_orphans(standing_tiles)) {
        fan_table[THIRTEEN_ORPHANS] = 1;
    }
    // 全不靠/七星不靠
    else if (calculate_honors_and_knitted_tiles(standing_tiles, fan_table)) {
    }
    else {
        return false;
    }

    adjust_by_win_flag(win_flag, fan_table);
    // 根据风调整就没必要了，这些特殊和型都没有面子，不存在圈风刻、门风刻
    // 统一调整一些不计的
    adjust_fan_table(fan_table, false);
    return true;
}

// 从番表计算番数
static int get_fan_by_table(const fan_table_t &fan_table) {
    int fan = 0;
    for (int i = 1; i < FAN_TABLE_SIZE; ++i) {
        if (fan_table[i] == 0) {
            continue;
        }
        fan += fan_value_table[i] * fan_table[i];
#if 0  // Debug
        if (fan_table[i] == 1) {
            LOG("%s %hu\n", fan_name[i], fan_value_table[i]);
        }
        else {
            LOG("%s %hu*%hu\n", fan_name[i], fan_value_table[i], fan_table[i]);
        }
#endif
    }
    return fan;
}

// 判断立牌是否包含和牌
bool is_standing_tiles_contains_win_tile(const tile_t *standing_tiles, intptr_t standing_cnt, tile_t win_tile) {
    return std::any_of(standing_tiles, standing_tiles + standing_cnt,
        [win_tile](tile_t tile) { return tile == win_tile; });
}

// 统计和牌在副露牌组中出现的张数
size_t count_win_tile_in_fixed_packs(const pack_t *fixed_packs, intptr_t fixed_cnt, tile_t win_tile) {
    tile_table_t cnt_table = { 0 };
    for (intptr_t i = 0; i < fixed_cnt; ++i) {
        pack_t pack = fixed_packs[i];
        tile_t tile = pack_get_tile(pack);
        switch (pack_get_type(pack)) {
        case PACK_TYPE_CHOW: ++cnt_table[tile - 1]; ++cnt_table[tile]; ++cnt_table[tile + 1]; break;
        case PACK_TYPE_PUNG: cnt_table[tile] += 3; break;
        case PACK_TYPE_KONG: cnt_table[tile] += 4; break;
        default: break;
        }
    }
    return cnt_table[win_tile];
}

// 判断副露牌组是否包含杠
bool is_fixed_packs_contains_kong(const pack_t *fixed_packs, intptr_t fixed_cnt) {
    return std::any_of(fixed_packs, fixed_packs + fixed_cnt,
        [](pack_t pack) { return pack_get_type(pack) == PACK_TYPE_KONG; });
}

// 检查算番的输入是否合法
int check_calculator_input(const hand_tiles_t *hand_tiles, tile_t win_tile) {
    // 打表
    tile_table_t cnt_table;
    if (!map_hand_tiles(hand_tiles, &cnt_table)) {
        return ERROR_WRONG_TILES_COUNT;
    }
    if (win_tile != 0) {
        ++cnt_table[win_tile];
    }

    // 如果某张牌超过4
    if (std::any_of(std::begin(cnt_table), std::end(cnt_table), [](int cnt) { return cnt > 4; })) {
        return ERROR_TILE_COUNT_GREATER_THAN_4;
    }

    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// 算番
//
int calculate_fan(const calculate_param_t *calculate_param, fan_table_t *fan_table) {
    const hand_tiles_t *hand_tiles = &calculate_param->hand_tiles;
    tile_t win_tile = calculate_param->win_tile;
    win_flag_t win_flag = calculate_param->win_flag;

    if (int ret = check_calculator_input(hand_tiles, win_tile)) {
        return ret;
    }

    intptr_t fixed_cnt = hand_tiles->pack_count;
    intptr_t standing_cnt = hand_tiles->tile_count;

    // 校正和牌标记
    // 如果立牌包含和牌，则必然不是和绝张
    const bool standing_tiles_contains_win_tile = is_standing_tiles_contains_win_tile(hand_tiles->standing_tiles, standing_cnt, win_tile);
    if (standing_tiles_contains_win_tile) {
        win_flag &= ~WIN_FLAG_4TH_TILE;
    }

    // 如果和牌在副露中出现3张，则必然为和绝张
    const size_t win_tile_in_fixed_packs = count_win_tile_in_fixed_packs(hand_tiles->fixed_packs, fixed_cnt, win_tile);
    if (3 == win_tile_in_fixed_packs) {
        win_flag |= WIN_FLAG_4TH_TILE;
    }

    // 附加杠标记
    if (win_flag & WIN_FLAG_ABOUT_KONG) {
        if (win_flag & WIN_FLAG_SELF_DRAWN) {  // 自摸
            // 如果手牌没有杠，则必然不是杠上开花
            if (!is_fixed_packs_contains_kong(hand_tiles->fixed_packs, fixed_cnt)) {
                win_flag &= ~WIN_FLAG_ABOUT_KONG;
            }
        }
        else {  // 点和
            // 如果和牌在手牌范围内出现过，则必然不是抢杠和
            if (win_tile_in_fixed_packs > 0 || standing_tiles_contains_win_tile) {
                win_flag &= ~WIN_FLAG_ABOUT_KONG;
            }
        }
    }

    // 合并立牌与和牌，并排序，最多为14张
    tile_t standing_tiles[14];
    memcpy(standing_tiles, hand_tiles->standing_tiles, standing_cnt * sizeof(tile_t));
    standing_tiles[standing_cnt] = win_tile;
    std::sort(standing_tiles, standing_tiles + standing_cnt + 1);

    // 最大番标记
    int max_fan = 0;
    const fan_table_t *selected_fan_table = nullptr;

    // 特殊和型的番
    fan_table_t special_fan_table = { 0 };

    // 先判断各种特殊和型
    if (fixed_cnt == 0) {  // 门清状态，有可能是基本和型组合龙
        if (calculate_knitted_straight_fan(calculate_param, special_fan_table)) {
            max_fan = get_fan_by_table(special_fan_table);
            selected_fan_table = &special_fan_table;
            LOG("fan = %d\n\n", max_fan);
        }
        else if (calculate_special_form_fan(standing_tiles, win_flag, special_fan_table)) {
            max_fan = get_fan_by_table(special_fan_table);
            selected_fan_table = &special_fan_table;
            LOG("fan = %d\n\n", max_fan);
        }
    }
    else if (fixed_cnt == 1) {  // 1副露状态，有可能是基本和型组合龙
        if (calculate_knitted_straight_fan(calculate_param, special_fan_table)) {
            max_fan = get_fan_by_table(special_fan_table);
            selected_fan_table = &special_fan_table;
            LOG("fan = %d\n\n", max_fan);
        }
    }

    // 无法构成特殊和型或者为七对
    // 七对也要按基本和型划分，因为极端情况下，基本和型的番会超过七对的番
    if (selected_fan_table == nullptr || special_fan_table[SEVEN_PAIRS] == 1) {
        // 划分
        division_result_t result;
        if (divide_win_hand(standing_tiles, hand_tiles->fixed_packs, fixed_cnt, &result)) {
            fan_table_t fan_tables[MAX_DIVISION_CNT] = { { 0 } };

            // 遍历各种划分方式，分别算番，找出最大的番的划分方式
            for (intptr_t i = 0; i < result.count; ++i) {
#if 0  // Debug
                char str[64];
                packs_to_string(result.divisions[i].packs, 5, str, sizeof(str));
                puts(str);
#endif
                calculate_basic_form_fan(result.divisions[i].packs, calculate_param, fan_tables[i]);
                int current_fan = get_fan_by_table(fan_tables[i]);
                if (current_fan > max_fan) {
                    max_fan = current_fan;
                    selected_fan_table = &fan_tables[i];
                }
                LOG("fan = %d\n\n", current_fan);
            }
        }
    }

    if (selected_fan_table == nullptr) {
        return ERROR_NOT_WIN;
    }

    // 加花牌
    max_fan += calculate_param->flower_count;

    if (fan_table != nullptr) {
        memcpy(*fan_table, *selected_fan_table, sizeof(*fan_table));
        (*fan_table)[FLOWER_TILES] = calculate_param->flower_count;
    }

    return max_fan;
}

}

using namespace std;

static map<string, mahjong::tile_t> str2tile;

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
    int quanFeng)
{
    vector<pair<int,string>> ans;
    mahjong::calculate_param_t calculate_param;
    mahjong::fan_table_t fan_table;
    memset(&calculate_param, 0, sizeof(mahjong::calculate_param_t));
    memset(&fan_table, 0, sizeof(mahjong::fan_table_t));
    calculate_param.hand_tiles.tile_count = hand.size();
    for(unsigned int i = 0; i < hand.size(); i++) {
        if(str2tile.find(hand[i]) == str2tile.end()){
            throw string("ERROE_WRONG_TILE_CODE");
        }
        calculate_param.hand_tiles.standing_tiles[i] = str2tile[hand[i]];
    }
    calculate_param.hand_tiles.pack_count = pack.size();
    for(unsigned int i = 0; i < pack.size(); i++) {
        pair<string, pair<string, int>> &sPack = pack[i];
        mahjong::pack_t &dPack = calculate_param.hand_tiles.fixed_packs[i];
        if(sPack.first == "PENG") {
            dPack = mahjong::make_pack(sPack.second.second, PACK_TYPE_PUNG, str2tile[sPack.second.first]);
        } else if(sPack.first == "GANG") {
            dPack = mahjong::make_pack(sPack.second.second, PACK_TYPE_KONG, str2tile[sPack.second.first]);
        } else if(sPack.first == "CHI"){
            dPack = mahjong::make_pack(sPack.second.second, PACK_TYPE_CHOW, str2tile[sPack.second.first]);
        } else {
            throw string("ERROE_WRONG_PACK_CODE");
        }
    }
    calculate_param.win_tile = str2tile[winTile];
    calculate_param.flower_count = flowerCount;
    if(isZIMO) {
        calculate_param.win_flag |= WIN_FLAG_SELF_DRAWN;
    }
    if(isLAST) {
        calculate_param.win_flag |= WIN_FLAG_WALL_LAST;
    }
    if(isJUEZHANG) {
        calculate_param.win_flag |= WIN_FLAG_4TH_TILE;
    }
    if(isGANG) {
        calculate_param.win_flag |= WIN_FLAG_ABOUT_KONG;
    }
    calculate_param.prevalent_wind = (mahjong::wind_t)quanFeng;
    calculate_param.seat_wind = (mahjong::wind_t)menFeng;
    int re = mahjong::calculate_fan(&calculate_param, &fan_table);
    if(re == -1) {
        throw string("ERROR_WRONG_TILES_COUNT");
    }else if(re == -2) {
        throw string("ERROR_TILE_COUNT_GREATER_THAN_4");
    }else if(re == -3) {
        throw string("ERROR_NOT_WIN");
    }
    for(int i = 0; i < mahjong::FAN_TABLE_SIZE; i++) {
        if(fan_table[i] > 0) {
            ans.push_back(make_pair(fan_table[i]*mahjong::fan_value_table[i],mahjong::fan_name[i]));
        }
    }
    return ans;
}

void MahjongInit()
{
    for(int i = 1; i <= 9; i++) {
        str2tile["W" + to_string(i)] = mahjong::make_tile(TILE_SUIT_CHARACTERS, i);
        str2tile["B" + to_string(i)] = mahjong::make_tile(TILE_SUIT_DOTS, i);
        str2tile["T" + to_string(i)] = mahjong::make_tile(TILE_SUIT_BAMBOO, i);
    }
    for(int i = 1; i <= 4; i++) {
        str2tile["F" + to_string((i))] = mahjong::make_tile(TILE_SUIT_HONORS, i);
    }
    for(int i = 1; i <= 3; i++) {
        str2tile["J" + to_string((i))] = mahjong::make_tile(TILE_SUIT_HONORS, i + 4);
    }
}
