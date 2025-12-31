#include <string.h>
#include "ebtn.h"

#define EBTN_FLAG_ONPRESS_SENT ((uint8_t)0x01)
#define EBTN_FLAG_IN_PROCESS ((uint8_t)0x02)

static ebtn_t ebtn_default;

static void prv_process_btn(ebtn_btn_t *btn, uint8_t old_state, uint8_t new_state, ebtn_time_t mstime)
{
    ebtn_t *ebtobj = &ebtn_default;

    if (btn->param == NULL)
    {
        return;
    }

    if (new_state != old_state)
    {
        btn->time_state_change = mstime;

        if (new_state)
        {
            btn->flags |= EBTN_FLAG_IN_PROCESS;
        }
    }

    if (new_state)
    {
        if (!(btn->flags & EBTN_FLAG_ONPRESS_SENT))
        {
            if (ebtn_timer_sub(mstime, btn->time_state_change) >= btn->param->time_debounce)
            {
                if ((btn->click_cnt > 0) && (ebtn_timer_sub(mstime, btn->click_last_time) >= btn->param->time_click_multi_max))
                {
                    if (btn->event_mask & EBTN_EVT_MASK_ONCLICK)
                    {
                        ebtobj->evt_fn(btn, EBTN_EVT_ONCLICK);
                    }
                    btn->click_cnt = 0;
                }

                btn->keepalive_last_time = mstime;
                btn->keepalive_cnt = 0;

                btn->flags |= EBTN_FLAG_ONPRESS_SENT;
                if (btn->event_mask & EBTN_EVT_MASK_ONPRESS)
                {
                    ebtobj->evt_fn(btn, EBTN_EVT_ONPRESS);
                }

                btn->time_change = mstime;
            }
        }
        else
        {
            while ((btn->param->time_keepalive_period > 0) && (ebtn_timer_sub(mstime, btn->keepalive_last_time) >= btn->param->time_keepalive_period))
            {
                btn->keepalive_last_time += btn->param->time_keepalive_period;
                ++btn->keepalive_cnt;
                if (btn->event_mask & EBTN_EVT_MASK_KEEPALIVE)
                {
                    ebtobj->evt_fn(btn, EBTN_EVT_KEEPALIVE);
                }
            }

            if ((btn->click_cnt > 0) && (ebtn_timer_sub(mstime, btn->time_change) > btn->param->time_click_pressed_max))
            {
                if (btn->event_mask & EBTN_EVT_MASK_ONCLICK)
                {
                    ebtobj->evt_fn(btn, EBTN_EVT_ONCLICK);
                }

                btn->click_cnt = 0;
            }
        }
    }
    else
    {
        if (btn->flags & EBTN_FLAG_ONPRESS_SENT)
        {
            if (ebtn_timer_sub(mstime, btn->time_state_change) >= btn->param->time_debounce_release)
            {
                btn->flags &= ~EBTN_FLAG_ONPRESS_SENT;
                if (btn->event_mask & EBTN_EVT_MASK_ONRELEASE)
                {
                    ebtobj->evt_fn(btn, EBTN_EVT_ONRELEASE);
                }

                if (ebtn_timer_sub(mstime, btn->time_change) >= btn->param->time_click_pressed_min &&
                    ebtn_timer_sub(mstime, btn->time_change) <= btn->param->time_click_pressed_max)
                {
                    ++btn->click_cnt;
                    btn->click_last_time = mstime;
                }
                else
                {
                    if ((btn->click_cnt > 0) && (ebtn_timer_sub(mstime, btn->time_change) < btn->param->time_click_pressed_min))
                    {
                        if (btn->event_mask & EBTN_EVT_MASK_ONCLICK)
                        {
                            ebtobj->evt_fn(btn, EBTN_EVT_ONCLICK);
                        }
                    }
                    btn->click_cnt = 0;
                }

                if ((btn->click_cnt > 0) && (btn->click_cnt == btn->param->max_consecutive))
                {
                    if (btn->event_mask & EBTN_EVT_MASK_ONCLICK)
                    {
                        ebtobj->evt_fn(btn, EBTN_EVT_ONCLICK);
                    }
                    btn->click_cnt = 0;
                }

                btn->time_change = mstime;
            }
        }
        else
        {
            if (btn->click_cnt > 0)
            {
                if (ebtn_timer_sub(mstime, btn->click_last_time) >= btn->param->time_click_multi_max)
                {
                    if (btn->event_mask & EBTN_EVT_MASK_ONCLICK)
                    {
                        ebtobj->evt_fn(btn, EBTN_EVT_ONCLICK);
                    }
                    btn->click_cnt = 0;
                }
            }
            else
            {
                if (btn->flags & EBTN_FLAG_IN_PROCESS)
                {
                    btn->flags &= ~EBTN_FLAG_IN_PROCESS;
                }
            }
        }
    }
}

int ebtn_init(ebtn_btn_t *btns, uint16_t btns_cnt, ebtn_btn_combo_t *btns_combo, uint16_t btns_combo_cnt, ebtn_get_state_fn get_state_fn, ebtn_evt_fn evt_fn)
{
    ebtn_t *ebtobj = &ebtn_default;

    if (evt_fn == NULL || get_state_fn == NULL)
    {
        return 0;
    }

    memset(ebtobj, 0x00, sizeof(*ebtobj));
    ebtobj->btns = btns;
    ebtobj->btns_cnt = btns_cnt;
    ebtobj->btns_combo = btns_combo;
    ebtobj->btns_combo_cnt = btns_combo_cnt;
    ebtobj->evt_fn = evt_fn;
    ebtobj->get_state_fn = get_state_fn;
    ebtobj->config = 0;

    return 1;
}

static void ebtn_get_current_state(bit_array_t *state_array)
{
    ebtn_t *ebtobj = &ebtn_default;
    ebtn_btn_dyn_t *target;
    int i;

    for (i = 0; i < ebtobj->btns_cnt; ++i)
    {
        uint8_t new_state = ebtobj->get_state_fn(&ebtobj->btns[i]);
        bit_array_assign(state_array, i, new_state);
    }

    for (target = ebtobj->btn_dyn_head, i = ebtobj->btns_cnt; target; target = target->next, i++)
    {
        uint8_t new_state = ebtobj->get_state_fn(&target->btn);
        bit_array_assign(state_array, i, new_state);
    }
}

static void ebtn_process_btn(ebtn_btn_t *btn, bit_array_t *old_state, bit_array_t *curr_state, int idx, ebtn_time_t mstime)
{
    prv_process_btn(btn, bit_array_get(old_state, idx), bit_array_get(curr_state, idx), mstime);
}

static void ebtn_process_btn_combo(ebtn_btn_t *btn, bit_array_t *old_state, bit_array_t *curr_state, bit_array_t *comb_key, ebtn_time_t mstime)
{
    BIT_ARRAY_DEFINE(tmp_data, EBTN_MAX_KEYNUM) = {0};

    if (bit_array_num_bits_set(comb_key, EBTN_MAX_KEYNUM) == 0)
    {
        return;
    }

    bit_array_and(tmp_data, curr_state, comb_key, EBTN_MAX_KEYNUM);
    uint8_t curr = bit_array_cmp(tmp_data, comb_key, EBTN_MAX_KEYNUM) == 0;

    bit_array_and(tmp_data, old_state, comb_key, EBTN_MAX_KEYNUM);
    uint8_t old = bit_array_cmp(tmp_data, comb_key, EBTN_MAX_KEYNUM) == 0;

    prv_process_btn(btn, old, curr, mstime);
}

void ebtn_process_with_curr_state(bit_array_t *curr_state, ebtn_time_t mstime)
{
    ebtn_t *ebtobj = &ebtn_default;
    ebtn_btn_dyn_t *target;
    ebtn_btn_combo_dyn_t *target_combo;
    int i;
    uint8_t combo_priority = ebtobj->config & EBTN_CFG_COMBO_PRIORITY;

    if (combo_priority)
    {
        bit_array_clear_all(ebtobj->combo_active, EBTN_MAX_KEYNUM);
    }

    if (combo_priority)
    {
        for (i = 0; i < ebtobj->btns_combo_cnt; ++i)
        {
            BIT_ARRAY_DEFINE(tmp_data, EBTN_MAX_KEYNUM) = {0};
            bit_array_t *comb_key = ebtobj->btns_combo[i].comb_key;

            bit_array_and(tmp_data, curr_state, comb_key, EBTN_MAX_KEYNUM);
            uint8_t curr = bit_array_cmp(tmp_data, comb_key, EBTN_MAX_KEYNUM) == 0;

            if (curr)
            {
                bit_array_or(ebtobj->combo_active, ebtobj->combo_active, comb_key, EBTN_MAX_KEYNUM);
            }
        }

        for (target_combo = ebtobj->btn_combo_dyn_head; target_combo; target_combo = target_combo->next)
        {
            BIT_ARRAY_DEFINE(tmp_data, EBTN_MAX_KEYNUM) = {0};
            bit_array_t *comb_key = target_combo->btn.comb_key;

            bit_array_and(tmp_data, curr_state, comb_key, EBTN_MAX_KEYNUM);
            uint8_t curr = bit_array_cmp(tmp_data, comb_key, EBTN_MAX_KEYNUM) == 0;

            if (curr)
            {
                bit_array_or(ebtobj->combo_active, ebtobj->combo_active, comb_key, EBTN_MAX_KEYNUM);
            }
        }
    }

    for (i = 0; i < ebtobj->btns_cnt; ++i)
    {
        if (combo_priority && bit_array_get(ebtobj->combo_active, i))
        {
            continue;
        }
        ebtn_process_btn(&ebtobj->btns[i], ebtobj->old_state, curr_state, i, mstime);
    }

    for (target = ebtobj->btn_dyn_head, i = ebtobj->btns_cnt; target; target = target->next, i++)
    {
        if (combo_priority && bit_array_get(ebtobj->combo_active, i))
        {
            continue;
        }
        ebtn_process_btn(&target->btn, ebtobj->old_state, curr_state, i, mstime);
    }

    for (i = 0; i < ebtobj->btns_combo_cnt; ++i)
    {
        ebtn_process_btn_combo(&ebtobj->btns_combo[i].btn, ebtobj->old_state, curr_state, ebtobj->btns_combo[i].comb_key, mstime);
    }

    for (target_combo = ebtobj->btn_combo_dyn_head; target_combo; target_combo = target_combo->next)
    {
        ebtn_process_btn_combo(&target_combo->btn.btn, ebtobj->old_state, curr_state, target_combo->btn.comb_key, mstime);
    }

    bit_array_copy_all(ebtobj->old_state, curr_state, EBTN_MAX_KEYNUM);
}

void ebtn_process(ebtn_time_t mstime)
{
    BIT_ARRAY_DEFINE(curr_state, EBTN_MAX_KEYNUM) = {0};
    ebtn_get_current_state(curr_state);
    ebtn_process_with_curr_state(curr_state, mstime);
}

int ebtn_get_total_btn_cnt(void)
{
    ebtn_t *ebtobj = &ebtn_default;
    int total_cnt = 0;
    ebtn_btn_dyn_t *curr = ebtobj->btn_dyn_head;

    total_cnt += ebtobj->btns_cnt;

    while (curr)
    {
        total_cnt++;
        curr = curr->next;
    }
    return total_cnt;
}

int ebtn_get_btn_index_by_key_id(uint16_t key_id)
{
    ebtn_t *ebtobj = &ebtn_default;
    int i = 0;
    ebtn_btn_dyn_t *target;

    for (i = 0; i < ebtobj->btns_cnt; ++i)
    {
        if (ebtobj->btns[i].key_id == key_id)
        {
            return i;
        }
    }

    for (target = ebtobj->btn_dyn_head, i = ebtobj->btns_cnt; target; target = target->next, i++)
    {
        if (target->btn.key_id == key_id)
        {
            return i;
        }
    }

    return -1;
}

ebtn_btn_t *ebtn_get_btn_by_key_id(uint16_t key_id)
{
    ebtn_t *ebtobj = &ebtn_default;
    int i = 0;
    ebtn_btn_dyn_t *target;

    for (i = 0; i < ebtobj->btns_cnt; ++i)
    {
        if (ebtobj->btns[i].key_id == key_id)
        {
            return &ebtobj->btns[i];
        }
    }

    for (target = ebtobj->btn_dyn_head, i = ebtobj->btns_cnt; target; target = target->next, i++)
    {
        if (target->btn.key_id == key_id)
        {
            return &target->btn;
        }
    }

    return NULL;
}

int ebtn_get_btn_index_by_btn(ebtn_btn_t *btn)
{
    return ebtn_get_btn_index_by_key_id(btn->key_id);
}

int ebtn_get_btn_index_by_btn_dyn(ebtn_btn_dyn_t *btn)
{
    return ebtn_get_btn_index_by_key_id(btn->btn.key_id);
}

void ebtn_combo_btn_add_btn_by_idx(ebtn_btn_combo_t *btn, int idx)
{
    bit_array_set(btn->comb_key, idx);
}

void ebtn_combo_btn_remove_btn_by_idx(ebtn_btn_combo_t *btn, int idx)
{
    bit_array_clear(btn->comb_key, idx);
}

void ebtn_combo_btn_add_btn(ebtn_btn_combo_t *btn, uint16_t key_id)
{
    int idx = ebtn_get_btn_index_by_key_id(key_id);
    if (idx < 0)
    {
        return;
    }
    ebtn_combo_btn_add_btn_by_idx(btn, idx);
}

void ebtn_combo_btn_remove_btn(ebtn_btn_combo_t *btn, uint16_t key_id)
{
    int idx = ebtn_get_btn_index_by_key_id(key_id);
    if (idx < 0)
    {
        return;
    }
    ebtn_combo_btn_remove_btn_by_idx(btn, idx);
}

int ebtn_is_btn_active(const ebtn_btn_t *btn)
{
    return btn != NULL && (btn->flags & EBTN_FLAG_ONPRESS_SENT);
}

int ebtn_is_btn_in_process(const ebtn_btn_t *btn)
{
    return btn != NULL && (btn->flags & EBTN_FLAG_IN_PROCESS);
}

int ebtn_is_in_process(void)
{
    ebtn_t *ebtobj = &ebtn_default;
    ebtn_btn_dyn_t *target;
    ebtn_btn_combo_dyn_t *target_combo;
    int i;

    for (i = 0; i < ebtobj->btns_cnt; ++i)
    {
        if (ebtn_is_btn_in_process(&ebtobj->btns[i]))
        {
            return 1;
        }
    }

    for (target = ebtobj->btn_dyn_head, i = ebtobj->btns_cnt; target; target = target->next, i++)
    {
        if (ebtn_is_btn_in_process(&target->btn))
        {
            return 1;
        }
    }

    for (i = 0; i < ebtobj->btns_combo_cnt; ++i)
    {
        if (ebtn_is_btn_in_process(&ebtobj->btns_combo[i].btn))
        {
            return 1;
        }
    }

    for (target_combo = ebtobj->btn_combo_dyn_head; target_combo; target_combo = target_combo->next)
    {
        if (ebtn_is_btn_in_process(&target_combo->btn.btn))
        {
            return 1;
        }
    }

    return 0;
}

int ebtn_register(ebtn_btn_dyn_t *button)
{
    ebtn_t *ebtobj = &ebtn_default;

    ebtn_btn_dyn_t *curr = ebtobj->btn_dyn_head;
    ebtn_btn_dyn_t *last = NULL;

    if (!button)
    {
        return 0;
    }

    if (ebtn_get_total_btn_cnt() >= EBTN_MAX_KEYNUM)
    {
        return 0;
    }

    if (curr == NULL)
    {
        ebtobj->btn_dyn_head = button;
        button->next = NULL;
        return 1;
    }

    while (curr)
    {
        if (curr == button)
        {
            return 0;
        }
        last = curr;
        curr = curr->next;
    }

    last->next = button;
    button->next = NULL;

    return 1;
}

int ebtn_combo_register(ebtn_btn_combo_dyn_t *button)
{
    ebtn_t *ebtobj = &ebtn_default;

    ebtn_btn_combo_dyn_t *curr = ebtobj->btn_combo_dyn_head;
    ebtn_btn_combo_dyn_t *last = NULL;

    if (!button)
    {
        return 0;
    }

    if (curr == NULL)
    {
        ebtobj->btn_combo_dyn_head = button;
        button->next = NULL;
        return 1;
    }

    while (curr)
    {
        if (curr == button)
        {
            return 0;
        }
        last = curr;
        curr = curr->next;
    }

    last->next = button;
    button->next = NULL;

    return 1;
}

void ebtn_set_config(uint8_t cfg_flags)
{
    ebtn_t *ebtobj = &ebtn_default;
    ebtobj->config = cfg_flags;
}

uint8_t ebtn_get_config(void)
{
    ebtn_t *ebtobj = &ebtn_default;
    return ebtobj->config;
}
