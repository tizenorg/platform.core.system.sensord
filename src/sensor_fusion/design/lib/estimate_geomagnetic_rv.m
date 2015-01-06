% estimate_gravity
%
% Copyright (c) 2014 Samsung Electronics Co., Ltd.
%
% Licensed under the Apache License, Version 2.0 (the "License");
% you may not use this file except in compliance with the License.
% You may obtain a copy of the License at
%
% http://www.apache.org/licenses/LICENSE-2.0
%
% Unless required by applicable law or agreed to in writing, software
% distributed under the License is distributed on an "AS IS" BASIS,
% WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
% See the License for the specific language governing permissions and
% limitations under the License.

% Gravitational Force Estimation function
%
% - Orientation Estimation using estimate_orientation function
% - Project the orientation on the device reference axes to obtain
%   gravitational force on specific reference axes


function [Quat_aid]  = estimate_geomagnetic_rv(Accel_data, Mag_data)

GRAVITY = 9.80665;
RAD2DEG = 57.2957795;

PITCH_PHASE_CORRECTION = -1;
ROLL_PHASE_CORRECTION = -1;
YAW_PHASE_CORRECTION = -1;

BUFFER_SIZE = size(Accel_data,2);

Gravity = zeros(3,BUFFER_SIZE);

OR_driv = zeros(3,BUFFER_SIZE);
Gyro_data = zeros(3,BUFFER_SIZE);

Quat_driv = zeros(4,BUFFER_SIZE);
Quat_aid = zeros(4,BUFFER_SIZE);
Quat_err = zeros(4,BUFFER_SIZE);

% estimate orientation
[Quat_driv, Quat_aid, Quat_err]  = estimate_orientation(Accel_data, Gyro_data, Mag_data);

end
