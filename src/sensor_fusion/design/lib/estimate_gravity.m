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


function [Gravity]  = estimate_gravity(Accel_data, Gyro_data, Mag_data)

GRAVITY = 9.80665;

BUFFER_SIZE = size(Accel_data,2);

OR_driv = zeros(3,BUFFER_SIZE);
OR_aid = zeros(3,BUFFER_SIZE);
OR_err = zeros(3,BUFFER_SIZE);

Gravity = zeros(3,BUFFER_SIZE);

% estimate orientation
[OR_driv, OR_aid, OR_err]  = estimate_orientation(Accel_data, Gyro_data, Mag_data);

Gx = GRAVITY * sind(OR_driv(1,:));
Gy = GRAVITY * sind(OR_driv(2,:));
Gz = GRAVITY * cosd(OR_driv(2,:)) .* cosd(OR_driv(1,:));

Gravity = [Gx; Gy; Gz];

end