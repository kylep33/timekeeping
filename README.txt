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
FACE_SETTINGS, FACE_VOLTAGE

this is configuration and settings etc.

    FACE_SET_TIME - set the time here.

    FACE_ADVANCED_ALARM - set your alarms here. you can set up to like 20. you
        can set different alarms for different days like mon-fri, different
        times, different loudness and different tones.

    FACE_FINETUNE - lines your seconds up with real time down to 25ms, and
        figures out how fast or slow your watch drifts. do the FACE_NANOSEC
        setup first (one time thing), then come back here every few weeks.
        - pull up time.is on your phone and watch the seconds on both
        - watch ahead? tap light = back 25ms. hold light = back 250ms
        - watch behind? tap strt = ahead 25ms. hold strt = ahead 250ms
        - keep going til the seconds flip at the exact same time. the big
          number is how much youve moved it in ms
        - tap mode to see hours since last finetune (DELtA). tap mode again
          to see the drift fix (Frq). needs 6+ hours or it just says 6HR
        - on Frq: hold light to apply the drift fix, or hold strt to skip
          it. both save and kick you back to the clock
        - very first time, skip it (hold strt on Frq). theres nothing to
          compare to yet
        - mode wont leave this face once youve moved the time. finish on Frq

    FACE_NANOSEC - the drift fixer. nudges the clock a tiny bit every 10 min
        and uses the temp sensor to fix drift from hot/cold too. end result
        is way less drift, goal is under a minute a year.
        one time setup:
        - tap mode til you see PROFL. tap light/strt to pick P3, then hold
          light to apply it. (P2 if P3 seems off)
        - tap mode to leave. thats it, finetune fills in FCorr for you from
          then on
        - other pages (CTMP 2Coef 3Coef Cadnc AgeCo) are nerd knobs. leave
          em alone
        - tap light +1, tap strt -1, hold for +/- 50
        - mode only leaves from the first page (FCorr), and saves on the way
          out

    FACE_VOLTAGE - shows your watch voltage. which should be in this range.
        ____ means its getting close to battery time change.
