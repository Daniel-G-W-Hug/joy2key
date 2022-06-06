### Joy2key - What is is all about (written by Daniel Hug 2022)

- joy2key is intended to translate keypresses on attached usb joysticks into virtual key presses
- the key presses represent commands (e.g. LShift + A, RCtrl + T, F10 or RAlt + HOME) that can be interpreted by simulation programs like e.g. DCS
- joy2key is intended to be run in the background to monitor the joysticks and initiate the key presses, while the simulation runs in the foreground as active window on your windows PC


### joy2key technical background:

- joysticks incl. their buttons are managed via DirectInput 8 (i.e. it is written for PCs running Windows exclusively)
- up to 8 joysticks can be handled and each can provide up to 128 virtual buttons and up to 4 pov hats
- each joystick has a unique identifier (GUID), can be assigned a joystick display name, a vendor ID and a product ID
- each virtual button models a two stage ON/OFF toggle, is numbered (starting with button 1) and can be assigned a button display name (default names "B1", "B2", ...)
- physical buttons might have two or more stages and can be modeled by several virtual toggle buttons, if required

condidered as an extension, but not yet implemented:
- in case POV hats are available, the 9 positions per hat (C, U, R, D, L, [UR, DR, DL, UL]) are mapped to virtual buttons (within the 128 virtual button limit). They are mapped to button numbers above the physically available buttons. There are max. 4 POV hats supported per joystick. Default names are "C_P1", "U_P1", ..., "UL_P1", ..., "C_P4", ..., "UL_P4".


### joy2key provides following core functionality:

- the user can configure the joystick names, the button names and the button to key press assignment via configuration files or interactively
- various profiles can be assigned to each joystick for various application purposes
- button presses can also be used to switch between profiles
- hotplugging of joysticks is supported
- the user can configure button modes as follows:

    1. **Immediate mode (default):**
    - A key press can be trigged when the button is toggled to on ("press action"; can given a descriptive name)
    - A key press can be trigged when the button is toggled to off ("release action"; can be given a descriptive name)
    - The key press is immediately generated for both cases as soon as the button toggle is registered by the program

    2. **Timed mode:**
    - A key press can be trigged when a button is pressed and released quickly (key press to initiate "short press action" is triggered at button release; can be given a descriptive name)
    - A key press can be trigged when the same button it is kept pressed for a longer time (key press to initiate "long press action" is triggered after a defined time when the button is kept pressed; can be given a descriptive name)
    - The the short press action is initiated when the button is released quickly after pressing the button. This happens at the release toggle when the button is released faster than "long press time" milliseconds after it has been toggled to on.
    - If the button in kept pressed for at least "long press time" milliseconds , an alternate action can be triggered with the same button. The button can then be released without initiating another action. Only after releasing the button the next action can be initiated with this button.
    - "long press time" has a default value for all buttons. The default for all buttons can be changed by the user as well as the value for each individual button, if required.