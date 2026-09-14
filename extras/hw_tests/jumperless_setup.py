"""Wire the RV8803 hardware-test fixture from Jumperless MicroPython.

Run on the Jumperless, not desktop Python. See jumperless.md for instructions.
Requires the Jumperless V5 MicroPython API (verified on firmware 5.7.11.0).
"""

import time
import jumperless as j

# RV8803 QT rev A: consecutive header pins, starting with VIN in row 1.
# Keep these Nano pins aligned with the existing hardware-test sketches.
# Connect VIN last; A0 must be its only power source for power-cycle tests.
CONNECTIONS = (
    (2, j.GND, "GND -> GND"),
    (3, j.A5, "SCL -> Nano A5"),
    (4, j.A4, "SDA -> Nano A4"),
    (5, j.D2, "CLOE -> Nano D2"),
    (6, j.D3, "INT -> Nano D3"),
    (7, j.D4, "EVI -> Nano D4"),
    (8, j.D5, "SQW -> Nano D5"),
    (1, j.A0, "VIN -> Nano A0"),
)
UART_CONNECTIONS = (
    (j.UART_TX, j.D0, "Jumperless TX -> Nano RX"),
    (j.UART_RX, j.D1, "Jumperless RX <- Nano TX"),
)

# Own only these endpoints. Other breadboard circuits and supply settings stay
# intact. In particular, remove old GPIO/ADC/power connections to our Nano pins.
# GND is shared, so never disconnect everything attached to the GND node.
OWNED_NODES = tuple(range(1, 9)) + (
    j.A0, j.A4, j.A5, j.D2, j.D3, j.D4, j.D5,
    j.D0, j.D1, j.UART_TX, j.UART_RX,
)


def setup():
    print("Adafruit RV8803 Jumperless fixture setup")
    print("Use a 5 V ATmega328P Nano in the Nano socket.")

    # Global connections remain active after this script/REPL exits.
    if j.context_get() != "global":
        j.context_toggle()
    if j.context_get() != "global":
        raise RuntimeError("Could not select persistent global connections")

    owned = {int(node) for node in OWNED_NODES}
    bridges = [j.get_bridge(i) for i in range(j.get_num_bridges())]
    print("Replacing these bridges (node1, node2, duplicates):")
    for bridge in bridges:
        if bridge[0] in owned or bridge[1] in owned:
            print(bridge)

    try:
        # The firmware's OLED service reserves Nano D2/D3 for its own I2C.
        # Ordinary disconnects are restored when leaving the REPL unless the
        # OLED service releases them. This fixture needs both pins for the RTC.
        j.oled_disconnect()
        print("OLED routing disabled to release Nano D2/D3.")

        # Break existing connections before adding any new ones, VIN first.
        for node in OWNED_NODES:
            j.disconnect(node, -1)

        for node1, node2, label in UART_CONNECTIONS + CONNECTIONS:
            j.connect(node1, node2)
            time.sleep(0.05)

        for node1, node2, label in UART_CONNECTIONS + CONNECTIONS:
            if not j.is_connected(node1, node2):
                raise RuntimeError("Missing bridge: " + label)
            if j.get_path_between(node1, node2) is None:
                raise RuntimeError("No routed path: " + label)
            print("PASS: " + label)

        # A requested bridge may exist alongside an unwanted supply or GPIO.
        # Check all bridges touching our endpoints, not just the desired pairs.
        expected = {
            tuple(sorted((int(a), int(b))))
            for a, b, label in UART_CONNECTIONS + CONNECTIONS
        }
        for i in range(j.get_num_bridges()):
            a, b, duplicates = j.get_bridge(i)
            if a in owned or b in owned:
                if tuple(sorted((a, b))) not in expected:
                    raise RuntimeError("Unexpected fixture bridge: %s-%s" % (a, b))
    except Exception:
        # Leave the RTC isolated if setup or routing fails, including VIN.
        for row in range(1, 9):
            j.disconnect(row, -1)
        raise

    print("Fixture routes verified. Hardware tests have not been run.")
    print("Upload a hardware-test sketch to the Nano; it enables VIN on A0.")


setup()
