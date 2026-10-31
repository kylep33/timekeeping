Hey!! readme.txt

here are your modes and how to move between then


================================================================================

The watch has these modes in this order
    1. daily_faces
    2. climb_faces
    3. game_faces
    4. pet_faces (turned off for julian)
    5. outdoor_faces
    6. setup_faces


================================================================================

breakdowns:

Standard control:
    - tap mode to switch faces.
    - hold mode to return to initial clock face

mode switch control
    - on init clock face hold mode
    - tap mode to cycle modes
    - wait 3s or hold mode to select.


================================================================================
1. daily_faces
================================================================================
FACE_CLOCK, FACE_COUNTDOWN, FACE_STOPWATCH, FACE_COIN_FLIP

    FACE CLOCK - always military time. deal with it

    FACE_COUNTDOWN - for 5 secnods after opening this face, each tap will add
        1 minute, you press start (bottom right). nifty!
        to reset this. pause the timer, then short press the light. it will reset. cycle back around to reactivate the 5s of tap

    FACE_STOPWATCH - a good stopwatch. to use it is  obvious

    FACE_COIN_FLIP - click the bottom right and it will flip heads or tails.
        cool


================================================================================
2. climb_faces
================================================================================
FACE_CLOCK, FACE_CLIMB_TIMER

simple on prupose. jsut a 24 hour clcok and climbertimer

    FACE_CLIMB_TIMER - will flash approach. push and hold to start appraoch
        timer. feel free to swtich to main face while its timing. tap
        strt/stop (bottom right) to start timer for next pitch. keep doing
        this up to 25 pitches.  push and hold when done. then you will be in
        summary mode. tap strt.stop to cycle through times. push and hold when
        ready to reset sum 0 is approach FYI


================================================================================
3. game_faces
================================================================================
FACE_CLOCK, FACE_PROBABILITY, FACE_ENDLESS_RUNNER, FACE_PING, FACE_TAROT,

    face_probabiltiy -  tap light to cycle options. press start to roll coin
        flip - d20 are options.

    FACE_ENDLESS_RUNNER, FACE_PING. simple games you can play

    FACE_TAROT - tarot card deck. no clue maybe your witchy forest friends can
        help


================================================================================
5. outdoor_faces (aka earth)
================================================================================
FACE_CLOCK, FACE_SUNRISE_SUNSET, FACE_MOON_PHASE

    FACE_SUNRISE_SUNSET - for preset locations tap the light, cycle between
        sd, yo(semite), uA(shington). sorry cant do a w very easy. if you use
        the strt button you can toggle through and set a lat/long for any
        location you want.

    FACE_MOON_PHASE - current moon cycles. go forward and back with  light ->
        strt/top button


================================================================================
6. setup_faces
================================================================================
FACE_CLOCK, FACE_SET_TIME, FACE_ADVANCED_ALARM, FACE_FINETUNE, FACE_NANOSEC,
FACE_SETTINGS, FACE_CHIME_SETTINGS, FACE_ALARM_SETTINGS, FACE_LED_SETTINGS,
FACE_VOLTAGE

this is configuration and settings etc.

    FACE_SET_TIME - set the time here.

    FACE_ADVANCED_ALARM - set your alarms here. you can set up to like 20. you
        can set different alarms for different days like mon-fri, different
        times, different loudness and different tones.

    FACE_FINETUNE - sync seconds to real time. do every few weeks.
        - open time.is on your phone
        - watch ahead: light (hold = bigger step). behind: strt
        - stop when the seconds flip together
        - tap mode twice to Frq. hold light = save. first time ever: hold
          strt instead

    FACE_NANOSEC - fixes drift. set up once, BEFORE finetune.
        - tap mode to PROFL. light/strt to P3. hold light
        - tap mode til it leaves. ignore the other pages

    settings faces: light = next page, strt = change it. settings with a *
    turn off on battery swap or reflash

    FACE_SETTINGS - CLOCK 12h/24h, BTN beep, TMOUt, LoEne, Build

    FACE_CHIME_SETTINGS
        - SIGNAL: chime volume
        - ANNOY*: chime every 15 min
        - MARI*: chime only 9am-10pm

    FACE_ALARM_SETTINGS
        - ALARM: alarm volume
        - MUTE*: no alarms ring. they stay set

    FACE_LED_SETTINGS - how long the light stays on, red/green/blue

    FACE_VOLTAGE - shows your watch voltage. which should be in this range.
        about 2.7V - 3.1V. the clock face also shows the two arrows icon when need change battery
