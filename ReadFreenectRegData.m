function RegData = ReadFreenectRegData(FileName)
%READFREENECTREGDATA - Imports Kinect Depth to RGB Registration Data
%   RegData = ReadFreenectRegData(FileName)
%       FileName - Name of the Kinect Registration file
%       RegData - Registration Data structure
%           .raw_to_mm_shift - Raw Data to Metric mapping
%           .depth_to_rgb_shift - Depth to RGB Shift mapping
%           .registration_table - Depth to RGB Registration mapping
%           .Error - Error Number
%           .ErrorMessage - Error Message
%
%   This software is furnished "as is", without technical support,
%   and with no warranty, express or implied, as to its usefulness for
%   any purpose.
%   Author: Sk. Mohammadul Haque
%   Copyright (c) 2014 Sk. Mohammadul Haque
%   Licensed under the Apache License, Version 2.0 (the "License");
%   you may not use this file except in compliance with the License.
%   You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
%   Unless required by applicable law or agreed to in writing, software distributed
%   under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
%   CONDITIONS OF ANY KIND, either express or implied. See the License for the
%   specific language governing permissions and limitations under the License.
%
%	For more details and updates, visit http://mohammadulhaque.alotspace.com
%

RegData = struct();
RegData.ErrorMessage = '';
RegData.Error = 0;
RegData.raw_to_mm_shift  = [];
RegData.depth_to_rgb_shift = [];
RegData.registration_table.x = [];
RegData.registration_table.y = [];

fp = fopen(FileName, 'rb');
if(fp==-1)
    RegData.ErrorMessage = 'Cannot Open File';
    RegData.Error = 1;
    fclose(fp);
    return;
end

try
    RegData.raw_to_mm_shift = uint16(fread(fp, 1024, 'uint16'));
catch me
    RegData.ErrorMessage = 'Error reading raw_to_mm_shift';
    RegData.Error = 2;
    fclose(fp);
    return;
end
try
    RegData.depth_to_rgb_shift = int32(fread(fp, 8192*2, 'int32'));
catch me
    RegData.ErrorMessage = 'Error reading depth_to_rgb_shift';
    RegData.Error = 3;
    fclose(fp);
    return;
end

try
    rtable = int32(fread(fp, 2*640*480, 'double'));
catch me
    RegData.ErrorMessage = 'Error reading registration_table';
    RegData.Error = 4;
    fclose(fp);
    return;
end

RegData.registration_table.x = reshape(double(rtable(2:2:end)), [640 480]).';
RegData.registration_table.y = reshape(double(rtable(1:2:end)), [640 480]).'/256;

end
