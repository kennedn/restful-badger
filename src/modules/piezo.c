#include <math.h>

#include "hardware/clocks.h"
#include "hardware/pwm.h"
#include "pico/stdlib.h"
#include "piezo.h"

// Note piezo_notes_test_1[] = {
//     {NOTE_C5, 50},
//     {NOTE_G4, 50},
//     {NOTE_C4, 50}
// };
const Note piezo_notes_test_1[] = {
    {NOTE_G4, 35},
    {NOTE_G5, 35},
    {NOTE_G6, 35}
};
size_t piezo_notes_test_1_len = count_of(piezo_notes_test_1);

const Note piezo_notes_test_2[] = {
    {NOTE_E6, 125},
    {NOTE_G6, 125},
    {NOTE_E7, 125},
    {NOTE_C7, 125},
    {NOTE_D7, 125},
    {NOTE_G7, 125}
};
size_t piezo_notes_test_2_len = count_of(piezo_notes_test_2);

const Note piezo_notes_test_3[] = {
    {NOTE_E5, 40},
    {NOTE_C5, 200},
    {NOTE_G4,300},
    {NOTE_C5, 200},
    {NOTE_G4,200},
    {NOTE_D5, 200},
    {NOTE_E5, 200},
    {NOTE_B5, 200},
    // {NOTE_C5, 200},
    // {NOTE_G4,200},
    // {NOTE_D5, 200},
    // {NOTE_F5, 200},
    // {NOTE_E5, 5000},
};
size_t piezo_notes_test_3_len = count_of(piezo_notes_test_3);

const Note piezo_notes_test_4[] = {
    // {NOTE_E5, 200},
    // {NOTE_NONE,80},
    // {NOTE_E5, 300},
    // {NOTE_C5, 1000},
    // {NOTE_NONE, 1000},
    // {NOTE_GS4, 200},
    // {NOTE_NONE,80},
    // {NOTE_GS4, 300},
    // {NOTE_F4, 1000},
    // {NOTE_NONE, 1000},
    // {NOTE_E5, 200},
    // {NOTE_NONE,80},
    // {NOTE_E5, 300},
    // {NOTE_C5, 1000},
    // {NOTE_NONE, 1000},
    // {NOTE_AS4, 200},
    // {NOTE_NONE,80},
    // {NOTE_AS4, 300},
    // {NOTE_C5, 1000},
    // {NOTE_NONE, 1000},
    {NOTE_C5, 200},
    {NOTE_G4,200},
    {NOTE_C5, 200},
    {NOTE_D5, 200},
    {NOTE_F5, 200},
    {NOTE_E5, 200},
    {NOTE_C5, 200},
    {NOTE_G4,300},
    
};
size_t piezo_notes_test_4_len = count_of(piezo_notes_test_4);

const Note piezo_notes_pause[] = {
    {NOTE_E6, 100},
    {NOTE_C6, 100},
    {NOTE_E6, 100},
    {NOTE_C6, 300},
};
size_t piezo_notes_pause_len =  count_of(piezo_notes_pause);
const Note piezo_notes_coin[] = {
    {NOTE_B5, 50},
    {NOTE_E6, 400},
};
size_t piezo_notes_coin_len =  count_of(piezo_notes_coin);


// const Note piezo_notes_ping[] = {
//     {NOTE_CS5, 100},
// };
const Note piezo_notes_ping[] = {
    {NOTE_GS6, 100},
    {NOTE_DS6, 200},
};
size_t piezo_notes_ping_len =  count_of(piezo_notes_ping);

// const Note piezo_notes_pong[] = {
//     {NOTE_A5, 100},
// };
const Note piezo_notes_pong[] = {
    {NOTE_AS5, 50},
    {NOTE_DS5, 50},
    {NOTE_AS5, 50},
    {NOTE_DS5, 50},
    {NOTE_AS5, 50},
    {NOTE_DS5, 50},
};
size_t piezo_notes_pong_len =  count_of(piezo_notes_pong);
const Note piezo_notes_pong_2[] = {
    {NOTE_GS4, 50},
    {NOTE_G4, 50},
    {NOTE_GS4, 50},
    {NOTE_G4, 50},
    {NOTE_GS4, 50},
    {NOTE_G4, 50},
};
size_t piezo_notes_pong_2_len =  count_of(piezo_notes_pong_2);

const Note piezo_notes_stop[] = {
    {NOTE_NONE, 0},
};
size_t piezo_notes_stop_len =  count_of(piezo_notes_stop);


typedef struct NoteQueue_ {
    uint pin;
    uint ptr;
    size_t notes_len;
    alarm_id_t id;
    const Note *notes;
    bool sustain;
} NoteQueue;

NoteQueue note_queue;

int64_t piezo_play_note(alarm_id_t id, void *data) {

    NoteQueue *note_queue = (NoteQueue *)data;
    Note note_duration = note_queue->notes[note_queue->ptr++];
    if (note_queue->ptr > note_queue->notes_len) {
        if (note_queue->sustain) {
            return 0;
        }
        note_duration = (Note){.frequency = NOTE_NONE, .duration = 0};
    }

    // This code finds a wrap value as close as possible to the max wrap value (2^16) to give the greatest
    // duty cycle resolution for a given freq_hz. This is achieved by scaling the clock frequency by a factor of 16
    gpio_set_function(note_queue->pin, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(note_queue->pin);
    pwm_config config = pwm_get_default_config();

    uint clock = clock_get_hz(clk_sys);
    uint clock_scaler = (1 << 16) / 16;
    float div = ceil((float)clock / (note_duration.frequency * clock_scaler)) / 16;
    // Cannot have a div less than 1
    div = (div < 1) ? 1 : div;
    float div_clock = clock / div;
    uint wrap = (uint)((div_clock) / note_duration.frequency) - 1;

    pwm_config_set_clkdiv(&config, div);
    pwm_config_set_wrap(&config, wrap);
    pwm_init(slice_num, &config, true);
    pwm_set_gpio_level(note_queue->pin, (uint)(0.5f * wrap));
    
    return note_duration.duration * 1000;
}

bool piezo_playing() {
    if (note_queue.notes_len == 0) {
        return false;
    }
    return !(note_queue.ptr > note_queue.notes_len);
}

void piezo_play(const Note *notes, size_t notes_len, bool sustain) {
    if (note_queue.id != -1) {
        cancel_alarm(note_queue.id);
    }

    note_queue = (NoteQueue) {
        .ptr = 0,
        .pin = PIEZO_PIN,
        .notes = notes,
        .notes_len = notes_len,
        .sustain = sustain
    };

    note_queue.id = add_alarm_in_ms(1, piezo_play_note, &note_queue, false);
}

void piezo_stop() {
    piezo_play(piezo_notes_stop, piezo_notes_stop_len, false);
}