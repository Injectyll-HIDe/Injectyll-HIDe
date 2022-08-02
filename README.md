
# Injectyll-HIDe

HID implant that allows for key recording and injection over a Digimesh RF network.

## Authors

- Jonathan Fischer - [@c4m0ufl4g3](https://mobile.twitter.com/c4m0ufl4g3)
- Jeremy Miller - [@allTheJurm](https://mobile.twitter.com/allTheJurm)


## Contributing

Contributions are always welcome!

Feel free to ask us in our Discord or submit a Pull Request/issue to modifications or improvements. If you have a feature you believe should be forked from this, feel free to submit as a revision.


## Demo

<https://www.youtube.com/channel/UCUG-gIV2QooQGeMTGCVMTpQ>

## License

[MIT](https://choosealicense.com/licenses/mit/)


## Running the C2 (ihide.py)


You will need a USB interface for the C2 device to connect the radio to your computer. Sparkfun sells on here:
 https://www.sparkfun.com/products/11697


Install the digi-xbee library so the python script can interface with the radio along with additional dependencies.

Currently the Digi Python Library only works with Python v3.8.x, Python 3.9.x will not handle the libraries correctly

```bash
  pip install digi-xbee
  pip install blessed

```

With your xbee radio plugged in, review your device manager to see what COM port the xbee radio is connected to.

Now run the python script
```bash
  python ihide.py

```

When you run the python file you will need to select the COM port and baud rate (default is 9600)


## Implant Installation

Pending...
## Support

For support, join our discord or submit an issue to this repo.

Discord: https://discord.gg/uxzFeKnwdF


