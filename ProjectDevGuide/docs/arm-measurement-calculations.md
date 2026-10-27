**Arm Measurement Calculations**  
Source: [Arm_IK_Measurements.xlsx](Arm_IK_Measurements.xlsx) — reviewed update-7, renamed as the main workbook.  
   
 Purpose: reproduce the workbook geometry numbers, then show worked forward and inverse kinematics for the  
   
 reduced planar model plus the full 5-joint model structure. It is not a complete IK solver and not a  
   
 calibration procedure.  
   
 All lengths below are in **mm**, as labelled in the workbook. Angles are in **degrees** unless marked  
   
 **rad**.  
**1. Inputs**  
Axes columns G, H, I hold CAD coordinates. H is the CAD Y (up) component used by B4, B7, B8, B9.  
  
|  |  |  |  |  |  
| - | - | - | - | - |  
| **Point** |  **Row** |  **G (CAD X)** |  **H (CAD Y)** |  **I (CAD Z)** |  
| J1 |  5 |  0 |  11.27 |  18.18 |  
| J2 |  6 |  -23.15 |  59.72 |  21.08 |  
| J3 |  7 |  -23.14 |  172.84 |  14.18 |  
| J4 |  8 |  -23.14 |  269.09 |  33.43 |  
| J5 |  9 |  -23.14 |  299.24 |  19.08 |  
  
   
Other inputs:  
- `Axes!H14 = -30.5`: support/floor surface (bottom of base_link). The arm base rests directly on the floor, no spacer (user confirmation).  
- `Axes!H15 = 409.12`: left jaw-tip Y position.  
- `Axes!H16 = 407.12`: right jaw-tip Y position.  
- `Real Arm Check!B3 = 2`: allowed difference.  
- `IK Lengths!B11 = 3.5`: setback from jaw tips toward J5 to the object centre.  
**2. Position vs difference**  
A **position** is one coordinate. A **length** is a difference between two positions.  
```  
length = p2_axis - p1_axis  
 H14 = -30.5 is a negative coordinate, not a negative length.  
 J2 H - table H = 59.72 - (-30.5) = 59.72 + 30.5 = 90.22 mm  
   
```  
Keep the sign. Use `ABS()` only where the formula defines a lateral offset.  
   
 The support/floor is 30.5 mm below the CAD origin. H is the spreadsheet column for CAD Y.  
   
 The CAD floor-to-J2 value is 90.22 mm; the measured value is 90.3 mm. Keep them separate.  
**3. CAD lengths B4-B11**  
Read the rows in order. Output cells B4-B11 belong to **IK Lengths**.  
  
|  |  |  |  |  |  
| - | - | - | - | - |  
| **Cell** |  **Formula** |  **Substitution** |  **Value** |  **Projection** |  
| B4 |  `Axes!H6 - Axes!H14` |  `59.72 - (-30.5)` |  90.22 |  H: table -> J2 |  
| B5 |  `ABS(Axes!G6 - Axes!G5)` |  `ABS(-23.15 - 0)` |  23.15 |  G: J1 -> J2 |  
| B6 |  `ABS(Axes!I8 - Axes!I5)` |  `ABS(33.43 - 18.18)` |  15.25 |  I: J1 -> J4 |  
| B7 |  `Axes!H7 - Axes!H6` |  `172.84 - 59.72` |  113.12 |  H: J2 -> J3 |  
| B8 |  `Axes!H9 - Axes!H7` |  `299.24 - 172.84` |  126.40 |  H: J3 -> J5 |  
| B9 |  `AVERAGE(Axes!H15:H16) - Axes!H9` |  `(409.12+407.12)/2 - 299.24` |  108.88 |  H: J5 -> mean tip |  
| B11 |  `0.35 cm * 10` |  `0.35 * 10` |  3.5 |  tip-to-centre setback |  
| B10 |  `B9 - B11` |  `108.88 - 3.5` |  105.38 |  corrected CAD tool |  
  
   
B9 in three steps:  
1. Add tip positions: `409.12 + 407.12 = 816.24 mm`.  
2. Divide by two: `816.24 / 2 = 408.12 mm`.  
3. Subtract J5 position: `408.12 - 299.24 = 108.88 mm`.  
**Important:** these are **reference-pose axis projections**, not arbitrary diagonal screw distances.  
   
 Each formula uses one coordinate difference, or the mean tip H value.  
   
 Do not replace B5, B6, B7 or B8 with a full 3D diagonal `sqrt(dG^2 + dH^2 + dI^2)`.  
**4. Final CAD / measured table**  
CAD cells are in **IK Lengths**. Real cells and tolerance B3 are in **Real Arm Check**.  
```  
D = Real - CAD  
 if ABS(D) > B3: CHECK  
 else:           OK  
 B3 = 2 mm  
   
```  
  
|  |  |  |  |  |  |  |  
| - | - | - | - | - | - | - |  
| **Quantity** |  **CAD cell** |  **CAD (mm)** |  **Real cell** |  **Real (mm)** |  **D = Real - CAD** |  **Check** |  
| Table -> J2 shoulder |  B4 |  90.22 |  C6 |  90.3 |  0.08 |  OK |  
| Upper arm |  B7 |  113.12 |  C7 |  114 |  0.88 |  OK |  
| Forearm |  B8 |  126.40 |  C8 |  127 |  0.60 |  OK |  
| Tip reference |  B9 |  108.88 |  C9 |  107 |  -1.88 |  OK |  
  
   
All four pass: `ABS(D) <= 2`. Largest absolute difference is 1.88 mm.  
   
 Sign matters: C9 is lower than B9, so D is negative. `-1.88` is still inside tolerance.  
**Reference-pose sums**  
```  
CAD (using CAD B10):  
 90.22 + 113.12 + 126.40 + 105.38 = 435.12 mm  
   
 measured (using measured-derived tool 103.5):  
 90.3 + 114 + 127 + 103.5 = 434.8 mm  
   
```  
The measured sum is conditional on the tool alignment used for the 103.5 mm value.  
**Real derived tool value**  
```  
real tool = C9 - B11 = 107 - 3.5 = 103.5 mm  
 CAD B10  = 105.38 mm  
 CAD B10 - real derived tool = 105.38 - 103.5 = 1.88 mm  
   
```  
B10 uses CAD B9 minus B11, not the real measurement C9. It does not automatically become 103.5.  
   
 Keep B10 as the CAD reference. Use 103.5 mm as the measured-derived tool length for this grip.  
**5. CAD frame vs robot frame**  
CAD uses **Y up**. The robot frame is right-handed, with **x forward**, **y left** and **z up**:  
   
 `x × y = z`. A positive rotation follows the right-hand rule about its axis.  
The robot origin is the J1 axis intersection with the support/floor. The workbook CAD J1 axis is  
   
 `X=0, Z=18.18` and the support is `Y=-30.5`, so the robot origin in CAD coordinates is:  
```  
o = (0, -30.5, 18.18) mm  
   
```  
This uses workbook CAD references plus the user's confirmation that the base rests directly on the floor  
   
 (no spacer). It is not independent CAD verification.  
```  
x_robot = -X_cad  
 y_robot =  Z_cad - 18.18  
 z_robot =  Y_cad + 30.5  
   
 R = [ -1  0  0 ]  
     [  0  0  1 ]  
     [  0  1  0 ]  
   
 v_robot = R * v_cad          (direction vectors)  
 p_robot = R * (p_cad - o)    (positions)  
   
```  
**J2 in robot coordinates**  
```  
J2 in CAD: (X, Y, Z) = (-23.15, 59.72, 21.08) mm  
   
 x = -(-23.15)      = 23.15 mm  
 y = 21.08 - 18.18  =  2.90 mm  
 z = 59.72 + 30.5   = 90.22 mm  
   
 J2 robot CAD coordinate = (23.15, 2.90, 90.22) mm  
   
```  
`y = 2.90` is the J2 point's lateral position. It is not the 15.25 mm arm-plane offset B6  
   
 (`ABS(I8 - I5)`, an axis separation along robot Y). The measured floor-to-J2 is 90.3 mm;  
   
 CAD z is 90.22 mm, difference 0.08 mm (within B3).  
Position differences are origin-independent: `delta_p_robot = R * delta_p_cad`.  
   
 The transform preserves length, so B4-B11 can be compared as lengths; translate CAD points with the  
   
 formulas above before using them as robot coordinates.  
**6. Joint axes, angles and rotation signs**  
The robot origin is the J1 base axis at the support/floor: `x = y = z = 0`. The floor is `z = 0`; the  
   
 CAD origin is different. The frame is right-handed: `x` forward, `y` left, `z` up, `x × y = z`; a  
   
 positive joint angle follows the right-hand rule. `q1` is measured from `+x` toward `+y`. In the planar  
   
 model, `u` is forward radial distance; at `q1 = 0`, `u` is `+x`, and positive `d` is `+y`.  
Angles are degrees unless marked rad. Convert before `sin`/`cos` with `q_rad = q_deg * pi / 180`:  
   
 `30 deg = pi/6 = 0.5235988 rad`, `60 deg = pi/3 = 1.0471976 rad`, `-60 deg = -1.0471976 rad`.  
  
|  |  |  |  |  
| - | - | - | - |  
| **Joint** |  **Axis at straight-up reference pose** |  **Motion** |  **Positive model direction** |  
| J1 |  `+z` |  base yaw |  `+x` toward `+y` |  
| J2 |  `+y` |  shoulder pitch |  `+z` toward `+u` at `q1 = 0` |  
| J3 |  `+y` |  elbow pitch |  adds to forearm angle |  
| J4 |  `+z` |  wrist roll |  right-hand about `+z` |  
| J5 |  `+y` |  wrist pitch |  adds to tool angle at `q4 = 0` |  
  
   
These are model signs, not servo directions. At `q2 = q3 = q5 = 0` the links and tool point along `+z`;  
   
 that is a model reference, not a servo home. Verify the signs on the real arm.  
   
   
   
X \`Standard active rotations, with `c = cos(q)`, `s = sin(q)`, `q` in rad:  
```  
Rz(q) = [ c  -s   0 ]      Ry(q) = [  c   0   s ]  
         [ s   c   0 ]              [  0   1   0 ]  
         [ 0   0   1 ]              [ -s   0   c ]  
   
```  
`Rz(+90 deg)` maps `+x` to `+y`; `Ry(+90 deg)` maps `+z` to `+x`. Section 9 uses these definitions.  
**7. Reduced planar forward kinematics (q4 = 0)**  
Forward kinematics (FK) computes position from angles; inverse kinematics (IK) computes angles from a  
   
 target. This planar FK/IK is the only model with a fully worked inverse here. It assumes:  
- wrist roll fixed at `q4 = 0`;  
- the arm stays in one vertical plane apart from lateral offset `d`;  
- the tool has no additional lateral offset beyond `d`;  
- tool pitch `phi = q2 + q3 + q5`, measured from `+z` toward `+u`.  
The last point is why one tool angle works. With `q4 != 0`, roll moves the J5 pitch axis and this formula  
   
 is invalid. Values: `a = 23.15` (CAD-only forward), `d = 15.25` (CAD-only lateral, signed),  
   
 `h = 90.3` (measured floor to J2), `L1 = 114`, `L2 = 127`, `Lt = 103.5`, `q4 = 0`. The result mixes  
   
 CAD-only and measured values; do not call it fully measured or fully CAD.  
```  
phi = q2 + q3 + q5  
   
 u = a + L1*sin(q2) + L2*sin(q2 + q3) + Lt*sin(phi)  
 v = h + L1*cos(q2) + L2*cos(q2 + q3) + Lt*cos(phi)  
   
 x = u*cos(q1) - d*sin(q1)  
 y = u*sin(q1) + d*cos(q1)  
 z = v  
   
```  
`u` is forward radial distance, `v` is height above the floor. The world equations rotate `(u, d)` by  
   
 `q1`; at `q1 = 0`, `x = u`, `y = d`.  
**Worked FK.** Input `q1 = 0, q2 = 30, q3 = 30, q4 = 0, q5 = -60` (degrees), so  
   
 `phi = 30 + 30 - 60 = 0`. With `sin30 = 0.5`, `cos30 = 0.8660254`, `sin60 = 0.8660254`,  
   
 `cos60 = 0.5`, `sin0 = 0`, `cos0 = 1`:  
```  
u = 23.15 + 114*0.5 + 127*0.8660254 + 103.5*0  
   = 23.15 + 57.000 + 109.985 + 0.000 = 190.135 mm  
   
 v = 90.3 + 114*0.8660254 + 127*0.5 + 103.5*1  
   = 90.3 + 98.727 + 63.500 + 103.500 = 356.027 mm  
   
 x = 190.135*1 - 15.25*0 = 190.135 mm  
 y = 190.135*0 + 15.25*1 =  15.250 mm  
 z = v                   = 356.027 mm  
   
```  
Computed target: **`(190.135, 15.250, 356.027) mm`**. Keep full precision during calculation;  
   
 the displayed values are rounded to three decimal places.  
**8. Reduced planar inverse kinematics (q4 = 0)**  
Input is target `(x, y, z)` **and desired tool pitch `phi`**, with fixed roll.  
   
 Output is one or more `(q1, q2, q3, q5)` sets; `q4 = 0`.  
**Base.**  
```  
r = sqrt(x*x + y*y)  
 u = sqrt(max(0, r*r - d*d))  
 q1 = atan2(y, x) - atan2(d, u)  
   
```  
This is the positive-forward branch `u >= 0`; it requires `r >= ABS(d)`. For `u >= 0`,  
   
 `atan2(d, u) = asin(d/r)`, the earlier limited base-angle form. `max(0, ...)` handles only tiny  
   
 negative round-off; a real `r < ABS(d)` target is unreachable.  
**Remove shoulder and tool offsets.**  
```  
u_w = u - a - Lt*sin(phi)  
 z_w = z - h - Lt*cos(phi)  
   
```  
**Elbow.**  
```  
C = (u_w*u_w + z_w*z_w - L1*L1 - L2*L2) / (2*L1*L2)  
 q3 = +/- acos(C)  
   
```  
The `+` and `-` values are the two elbow branches. `C` must be in `[-1, 1]`; otherwise the wrist point  
   
 is unreachable with these link lengths.  
**Shoulder and wrist**, for each valid `q3`:  
```  
q2 = atan2(u_w, z_w) - atan2(L2*sin(q3), L1 + L2*cos(q3))  
 q5 = phi - q2 - q3  
   
```  
Standard math libraries return radians from `atan2` and `acos`. Use radians in `sin`/`cos`  
   
 and keep `phi` in the same unit; convert final angles to degrees for display.  
**Worked IK** for `(190.135, 15.250, 356.027)`, `phi = 0`:  
```  
r = sqrt(190.135^2 + 15.250^2) = 190.746 mm  
 u = sqrt(190.746^2 - 15.250^2) = 190.135 mm  
 q1 = atan2(15.250, 190.135) - atan2(15.250, 190.135) = 0 deg  
   
 u_w = 190.135 - 23.15 - 103.5*sin(0) = 166.985 mm  
 z_w = 356.027 - 90.3 - 103.5*cos(0) = 162.227 mm  
   
 C = (166.985^2 + 162.227^2 - 114^2 - 127^2) / (2*114*127)  
   = (27884.066 + 26317.566 - 12996 - 16129) / 28956  
   ≈ 25076.632 / 28956 ≈ 0.8660254  
 q3 = acos(0.8660254) ≈ 30.000 deg  
   
 q2 = atan2(166.985, 162.227) - atan2(127*sin(30), 114 + 127*cos(30))  
    = 45.828 - atan2(63.500, 223.985)  
    = 45.828 - 15.828 = 30.000 deg  
 q5 = 0 - 30.000 - 30.000 = -60.000 deg  
   
```  
Result `(0, 30, 30, 0, -60) deg`, matching FK. The other elbow branch is `q3 = -30.000`,  
   
 `q2 = 61.656`, `q5 = -31.656` deg. Both are mathematical; neither is automatically valid on the arm.  
**Reachability and tolerance.** For the wrist, `rho = sqrt(u_w^2 + z_w^2)`. A real elbow solution needs  
   
 `ABS(L1 - L2) <= rho <= L1 + L2`, i.e. `13 mm <= rho <= 241 mm`; the example has `rho = 232.812 mm`.  
   
 Equivalently `-1 <= C <= 1`. Near a limit, round-off may give `C = 1.0000000001`; use a documented  
   
 tolerance, for example `1e-9` in double or `1e-5` in single precision, clamp only inside it, and  
   
 reject larger overshoot. Do not silently clamp an unreachable target.  
**Base singularities and branches.** `r >= ABS(d)`; the minimum base radius is `15.25 mm`. At `r = 0`,  
   
 `d` must also be zero and `q1` is undefined — the base singularity. At `r = ABS(d)`, `u = 0` and the  
   
 base branches meet; check this boundary separately. If `d = 0` and `r > 0`, then `q1 = atan2(y, x)`.  
   
 `sqrt(...)` selects positive `u`; a negative-`u` solution is a different branch and is not returned.  
`q3 = +/- acos(C)` gives two elbow branches. Equivalent angles differ by whole turns; select only  
   
 representatives inside calibrated joint limits. Never wrap blindly or clip a solution to fit. At `q3 = 0` or `pi` the elbow is  
   
 singular: small target changes can need large joint changes.  
**Limits and safety.** The equations ignore servo min/max, direction, collisions, cable wrap, speed and  
   
 the safety layer. Every solution must still pass those checks. This numeric example is a **math**  
   
 **example, not a safe motor command**; do not send it to the servos as a test.  
**9. Full 5-joint forward kinematics using fixed reference axes**  
With `q4 != 0`, the J5 pitch axis is no longer the fixed `+y` used by the planar model. Roll moves the  
   
 pitch axis, so `phi = q2 + q3 + q5` and the planar `u/v` equations do not describe the tool. General roll  
   
 needs the full rotation product; the planar formula does not handle it. This is the roll-before-pitch  
   
 wrist: J4 roll is proximal to J5 pitch.  
CAD-only axis points in robot coordinates, converted with section 5. `p1` may be any point on the J1  
   
 axis; the origin is chosen. These are axis points at the CAD reference pose, not measured centres or a  
   
 measured zero pose.  
  
|  |  |  |  
| - | - | - |  
| **Axis** |  **Point `p_i` (mm)** |  **Axis direction `w_i`** |  
| J1 |  `p1 = (0, 0, 0)` |  `w1 = (0, 0, 1)` |  
| J2 |  `p2 = (23.15, 2.90, 90.22)` |  `w2 = (0, 1, 0)` |  
| J3 |  `p3 = (23.14, -4.00, 203.34)` |  `w3 = (0, 1, 0)` |  
| J4 |  `p4 = (23.14, 15.25, 299.59)` |  `w4 = (0, 0, 1)` |  
| J5 |  `p5 = (23.14, 0.90, 329.74)` |  `w5 = (0, 1, 0)` |  
  
   
Check J4: CAD `(-23.14, 269.09, 33.43)` becomes  
   
 `(23.14, 33.43 - 18.18, 269.09 + 30.5) = (23.14, 15.25, 299.59)`.  
Directions are fixed in the base frame. For joint `i`, with `q_i` in rad:  
```  
R_i = R(w_i, q_i)                 # Rz for +z, Ry for +y  
 H_i = [ R_i        (I - R_i)*p_i ]  
       [ 0  0  0          1        ]  
 H_i * [p; 1] = p_i + R_i*(p - p_i)  
   
```  
`p_i` stays fixed; `I - R_i` keeps the axis point fixed. This is the fixed-axis  
   
 product-of-exponentials (POE) form.  
```  
p(q) = H1 * H2 * H3 * H4 * H5 * [pTCP0; 1]  
 R_tool(q) = R1 * R2 * R3 * R4 * R5 * R_tool_zero  
   
```  
`pTCP0` is the tool point at zero reference; at all `q = 0`, every `H_i = I`, so `p = pTCP0`. The  
   
 rightmost factor `H5` applies to the tool point first, then `H4`, and so on. In the physical chain, J4 is  
   
 proximal to J5; "rightmost first" is transform order, not joint order from the base. At `q4 = 0`,  
   
 `R2`, `R3`, `R5` combine to `Ry(q2 + q3 + q5) = Ry(phi)`, so tool orientation is  
   
 `Rz(q1) * Ry(phi) * R_tool_zero`. Planar position also requires the alignment assumptions in section 7;  
   
 zero roll alone does not remove small CAD offsets or an unknown tool offset.  
**Tool point and blocked numeric FK.** The workbook gives only jaw Y:  
```  
(409.12 + 407.12) / 2 = 408.12 mm  
 408.12 - 3.5 = 404.62 mm       # CAD Y of tool centre after B11  
 404.62 + 30.5 = 435.12 mm      # robot z  
   
```  
So `pTCP0 = (x_unknown, y_unknown, 435.12) mm`. Tool CAD X/Z and zero orientation are not supplied. Do  
   
 not fill the unknowns with J5 values or guesses; full numeric FK is blocked until they are supplied.  
   
 A matrix check is possible: with only `q4 = 30 deg`, `p4 = (23.14, 15.25, 299.59)` gives  
```  
H4 = [ 0.8660  -0.5000   0   10.7252 ]  
      [ 0.5000   0.8660   0   -9.5269 ]  
      [ 0        0        1     0     ]  
      [ 0        0        0     1     ]  
   
```  
This is not a tool position because `pTCP0` is incomplete. Once supplied: convert angles to rad, build  
   
 `R1..R5`, build `H1..H5`, multiply `H1 H2 H3 H4 H5`, apply to `[pTCP0; 1]`, and read position.  
   
 Orientation is `R1 R2 R3 R4 R5` times the unknown tool zero orientation.  
**10. Full inverse kinematics: numerical overview**  
No full closed-form inverse is claimed. The five positioning joints cannot produce an arbitrary 6-DOF  
   
 pose; the gripper is not a positioning joint. A solver should form a position residual (and orientation  
   
 residual if required), minimise its weighted square over `q1..q5` inside joint limits using bounded  
   
 least squares or a trust-region method, seed with the planar solution plus other starts, then recompute  
   
 the residual and reject failures, limit violations, collisions or safety violations. A numerical answer  
   
 is not automatically safe.  
**11. Servo command conversion**  
For a simple sign-and-zero mapping:  
```  
q = s * (servo - servo_zero)  
 servo = servo_zero + s * q  
   
```  
`s = +1` or `-1` when servo and joint angles are both in degrees. If command `c` uses counts and  
   
 `K` is counts per degree, use `c = c_zero + s*K*q`, or `q = s*(c-c_zero)/K`.  
   
 The zero, sign and scale are unknown until measured;  
   
 no numeric PWM value follows from geometry. Keep zero, home, safe min/max and direction separate.  
**12. Calibration checklist**  
1. Independently verify the origin `o`: J1 axis at CAD `X=0, Z=18.18`, support `Y=-30.5`, plus the  
   
 user's floor confirmation.  
2. Check CAD-only `a = 23.15`, `d = 15.25` and the full axis points on the real arm.  
3. Check the planar tool-alignment assumption before using its numeric results on the real arm.  
4. Record which jaw-tip reference was used for the real 107 mm measurement.  
5. Supply tool CAD X/Z and zero orientation before claiming full FK.  
6. Measure servo zero, home, safe min/max and direction separately from geometry.  
7. Test FK/IK on the host first. Disconnect servo signal wires for initial new-motion-code checks.  
**13. Assumptions and limits**  
- Values and formulas were read from the saved workbook during this session.  
- B4-B9 are reference-pose axis projections, not arbitrary 3D diagonals.  
- The planar model is valid only at fixed `q4 = 0`, with the assumed tool lateral alignment; it does  
   
 not handle general roll.  
- `a` and `d` are CAD-only and not physically verified. `h`, `L1`, `L2` are measured. `Lt` is  
   
 real-derived but depends on the tool alignment used for the 107 mm value.  
- Full axis points are CAD-only. `pTCP0` is incomplete, so full numeric FK is not available yet.  
- The numerical full IK is an overview, not an implemented or tested solver.  
- This document does not prove full geometry, calibration readiness or safe motor behaviour.  
