# Properties Window

This panel displays the properties of the selected object. Values update as soon
as editing finishes and the scroll area keeps long lists accessible. Properties
are grouped into categories such as **Transform** and **Appearance** so you can
collapse sections you rarely adjust. The **Object** section lists the object
**Role** rather than a fixed type. A drop-down lets you pick between the
built-in roles **empty**, **shape**, **sprite** and **camera**. There is no **Apply**
button – changes are stored immediately.

When no object is selected the panel is blank. Changing the role updates the
visible categories automatically so the Shape or Sprite options appear at once.

Rotation uses a circular dial that displays the angle value in degrees.
Position, scale and pivot use spin boxes with repeat buttons so holding the
arrows increments the values automatically.

Choosing the **shape** role reveals an additional **Shape** category
with a drop-down to select **square**, **triangle** or **circle**. Picking
the **sprite** role instead shows a **Sprite** category where you can
enter the image path and toggle texture filtering via the **Smooth** box.

## Physics

When the optional physics module is installed, objects include a **Physics**
category. Physics support is experimental. The tab contains two options:

- **Enabled** – toggles physics on or off for the object.
- **Body Type** – choose **Static** or **Dynamic** behaviour.
