function FData = LoadFreenectData(FileName)
%LOADFREENECTDATA - Loads Freenect Image Data from Raw File
%   FData = LoadFreenectImages(FileName)
%       FileName - Raw Freenect Data Filename
%       FData - File Data
%           .ErrorMessage -  Error Message
%           .Error - Error Number
%           .IRImage - IR Data
%           .RGBImage - RGB Data
%           .DepthImage - Depth Data
%           .RGBResolution - RGB Image Resolution
%           .IRResolution - IR Image Resolution
%           .RGBFormat - RGB Image Format
%           .IRFormat - IR Image Format
%           .DepthFormat - Depth Image Format
%           .Accelerometer - Accelerometer Reading
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


FData = struct();
FData.ErrorMessage = '';
FData.Error = 0;
FData.IRImage = [];
FData.RGBImage = [];
FData.DepthImage = [];
FData.RGBFormat = '';
FData.IRFormat = '';
FData.RGBBPP = 0;
FData.IRBPP = 0;
FData.DepthFormat = [];
FData.RGBResolution = [];
FData.IRResolution = [];
FData.Accelerometer = [];

fp = fopen(FileName, 'rb');
if(fp~=-1)
    fheader = fgets(fp, 4);
    if(~strcmp(fheader, 'FNK0')&&~strcmp(fheader, 'FNK1'))
        FData.ErrorMessage = 'File Format Mismatch';
        FData.Error = 1;
        fclose(fp);
        return;
    end

    %%/ FREENECT_RESOLUTION_MEDIUM is 640x488 for the IR camera.
    %typedef enum {
    %	FREENECT_RESOLUTION_LOW    = 0, /**< QVGA - 320x240 */
    %	FREENECT_RESOLUTION_MEDIUM = 1, /**< VGA  - 640x480 */
    %	FREENECT_RESOLUTION_HIGH   = 2, /**< SXGA - 1280x1024 */
    %} freenect_resolution;
    curr_rgb_res = uint8(fgets(fp, 1));
    switch(curr_rgb_res)
        case 255
            FData.RGBResolution = [];
        case 1
            FData.RGBResolution = [480 640];
        case 2
            FData.RGBResolution = [1024 1280];
        otherwise
            FData.ErrorMessage = 'RGB Resolution Mismatch';
            FData.Error = 2;
            fclose(fp);
            return;
    end

    curr_ir_res = uint8(fgets(fp, 1));
    switch(curr_ir_res)
        case 255
            FData.IRResolution = [];
        case 1
            FData.IRResolution = [480 640];
        case 2
            FData.IRResolution = [1024 1280];
        otherwise
            FData.ErrorMessage = 'IR Resolution Mismatch';
            FData.Error = 3;
            fclose(fp);
            return;
    end
    %typedef enum {
    %	FREENECT_VIDEO_RGB             = 0, /**< Decompressed RGB mode (demosaicing done by libfreenect) */
    %	FREENECT_VIDEO_BAYER           = 1, /**< Bayer compressed mode (raw information from camera) */
    %	FREENECT_VIDEO_IR_8BIT         = 2, /**< 8-bit IR mode  */
    %	FREENECT_VIDEO_IR_10BIT        = 3, /**< 10-bit IR mode */
    %	FREENECT_VIDEO_IR_10BIT_PACKED = 4, /**< 10-bit packed IR mode */
    %	FREENECT_VIDEO_YUV_RGB         = 5, /**< YUV RGB mode */
    %	FREENECT_VIDEO_YUV_RAW         = 6, /**< YUV Raw mode */
    %} freenect_video_format;

    curr_rgb_fmt = uint8(fgets(fp, 1));
    switch(curr_rgb_fmt)
        case 255
            FData.RGBFormat = '';
            FData.RGBBPP = 0;
        case 0
            FData.RGBFormat = 'FREENECT_VIDEO_RGB ';
            FData.RGBBPP = 24;
        case 5
            FData.RGBFormat = 'FREENECT_VIDEO_YUV_RGB';
            FData.RGBBPP = 24;
        otherwise
            FData.ErrorMessage = 'RGB Image Format Mismatch';
            FData.Error = 4;
            fclose(fp);
            return;
    end

    curr_ir_fmt = uint8(fgets(fp, 1));
    switch(curr_ir_fmt)
        case 255
            FData.IRFormat = '';
            FData.IRBPP = 0;
        case 2
            FData.IRFormat = 'FREENECT_VIDEO_IR_8BIT';
            FData.IRBPP = 8;
        case 3
            FData.IRFormat = 'FREENECT_VIDEO_IR_10BIT';
            FData.IRBPP = 16;
        otherwise
            FData.ErrorMessage = 'IR Image Format Mismatch';
            FData.Error = 5;
            fclose(fp);
            return;
    end
    
    %typedef enum {
    %	FREENECT_DEPTH_11BIT        = 0, /**< 11 bit depth information in one uint16_t/pixel */
    %	FREENECT_DEPTH_10BIT        = 1, /**< 10 bit depth information in one uint16_t/pixel */
    %	FREENECT_DEPTH_11BIT_PACKED = 2, /**< 11 bit packed depth information */
    %	FREENECT_DEPTH_10BIT_PACKED = 3, /**< 10 bit packed depth information */
    %	FREENECT_DEPTH_REGISTERED   = 4, /**< processed depth data in mm, aligned to 640x480 RGB */
    %	FREENECT_DEPTH_MM           = 5, /**< depth to each pixel in mm, but left unaligned to RGB image */
    %} freenect_depth_format;

    curr_depth_fmt = uint8(fgets(fp, 1));
    switch(curr_depth_fmt)
        case 255
            FData.DepthFormat = '';
        case 0
            FData.DepthFormat = 'FREENECT_DEPTH_11BIT';
        case 1
            FData.DepthFormat = 'FREENECT_DEPTH_10BIT';
        otherwise
            FData.ErrorMessage = 'Depth Format Mismatch';
            FData.Error = 6;
            fclose(fp);
            return;
    end
    
    
    content = uint8(fgets(fp, 1));
    bincontent = de2bi(content, 4);
    %  | x | RGB (3) | IR (2) | Depth (1) | - in reverse ordered
    try
        if(bincontent(1,3)==1)
            FData.RGBImage = uint8(reshape(fread(fp,3*prod(FData.RGBResolution), 'uint8'), [3*FData.RGBResolution(1,2) FData.RGBResolution(1,1)]).');
            FData.RGBImage = reshape([FData.RGBImage(:,1:3:end), FData.RGBImage(:,2:3:end), FData.RGBImage(:,3:3:end)], [FData.RGBResolution(1,1) FData.RGBResolution(1,2) 3]);
        end
        if(bincontent(1,2)==1)
            if(FData.IRBPP==8) 
                FData.IRImage = uint8(reshape(fread(fp, prod(FData.IRResolution), 'uint8'), [FData.IRResolution(1,2) FData.IRResolution(1,1)]).');
            else
                FData.IRImage = uint16(reshape(fread(fp, prod(FData.IRResolution), 'uint16'), [FData.IRResolution(1,2) FData.IRResolution(1,1)]).');
            end
        end

        if(bincontent(1,1)==1)
            if(strcmp(fheader, 'FNK0'))
                FData.DepthImage = (reshape(fread(fp, 640*480, 'ubit24'), [640 480]).');
            else
                FData.DepthImage = (reshape(fread(fp, 640*480, 'ubit16'), [640 480]).');
            end;
            FData.DepthImage(FData.DepthImage==2047) = 0;
        end
        
        if(~feof(fp))
            FData.Accelerometer = fread(fp, 3, 'double');
        end
        
        fclose(fp);
    catch me
        FData.ErrorMessage = me.message;
        FData.Error = 7;
        fclose(fp);
    end
else
    FData.ErrorMessage = 'Can not open File';
    FData.Error = 8;
end    
end
