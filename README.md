# example-standalone-inferencing-renesas-ekra8d1-pack

Static inference on Renesas EK-RA8D1 using Open CMSIS pack.

## How to import the Edge Impulse SDK and a model

1. Deploy your model as OPEN CMSIS pack
2. Unzip the content of the deployment which is:
    - The Edge Impulse SDK
    - Your model
3. Import them in e2studio, from Import→CMSIS pack
4. First import the SDK, then your model.
5. Then close and reopen e2studio to let the IDE load the new packages.
6. Now you need to include these packs in your project:
    - Open the configuration
    - Open the component tab
    - Include the pack you just imported, they should appear under EdgeImpulse
7. You can now use the Edge Impulse SDK and run your model on your Renesas target!

> [!NOTE]
> Remember that you also need to add the following CMSIS pack:
> - Core, version 5.9.0
> - DSP,  version 1.11.0 or 1.15.0
> - NN library, version 4.0.0 or 4.1.0

> [!NOTE]
> You need you provide the implementation for a series of function, see [Functions that require definition](https://docs.edgeimpulse.com/docs/run-inference/cpp-library/deploy-your-model-as-a-c-library#functions-that-require-definition) for a complete list.
