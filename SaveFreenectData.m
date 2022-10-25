function SaveFreenectData(FileName, RGBImage, IRImage, DepthImage, Accelerometer)
%SAVEFREENECTDATA Saves Freenect RAW adata in a file
%   SaveFreenectData(FileName, RGBImage, IRImage, DepthImage, Accelerometer)
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

if(nargin<5 ||isempty(Accelerometer)), Accelerometer = []; end
if(nargin<4 ||isempty(DepthImage)), DepthImage = []; end
if(nargin<3 ||isempty(IRImage)), IRImage = []; end
if(nargin<2 ||isempty(RGBImage)), RGBImage = []; end

    %%/ FREENECT_RESOLUTION_MEDIUM is 640x488 for the IR camera.
    %typedef enum {
    %	FREENECT_RESOLUTION_LOW    = 0, /**< QVGA - 320x240 */
    %	FREENECT_RESOLUTION_MEDIUM = 1, /**< VGA  - 640x480 */
    %	FREENECT_RESOLUTION_HIGH   = 2, /**< SXGA - 1280x1024 */
    %} freenect_resolution;

    if(~isempty(RGBImage))
        sz_rgb = size(RGBImage);
        format_rgb = 0;
        if(sz_rgb(1,1)==1024 && sz_rgb(1,2)==1280)
            resolution_rgb = 2;
        elseif(sz_rgb(1,1)==480 && sz_rgb(1,2)==640)
            resolution_rgb = 1;
        elseif(sz_rgb(1,1)==240 && sz_rgb(1,2)==320)
            error('Format currently not supported');
        else
            error('Invalid RGB Resolution.');
        end
    else
        format_rgb = 255;
        resolution_rgb = 255;
        sz_rgb = [];
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
    
    if(~isempty(IRImage))
        sz_ir = size(IRImage);
        if(sz_ir(1,1)==1024 && sz_ir(1,2)==1280)
            resolution_ir = 2;
            if(max(IRImage(:))>255)
                format_ir = 3;
            else
                format_ir = 2;
            end
        elseif(sz_ir(1,1)==480 && sz_ir(1,2)==640)
            resolution_ir = 1;
            format_ir = 2;
        else
            error('Invalid IR Resolution.');
        end
    else
        sz_ir = [];
        format_ir = 255;
        resolution_ir = 255;
    end
    
    format_depth = 0;
    if(~isempty(DepthImage))
        sz_depth = size(DepthImage);
        if(sz_depth(1,1)~=480 || sz_depth(1,2)~=640)
            error('Invalid Depth Resolution.');
        end
    else
        sz_depth = [];
    end
       
    if(~isempty(Accelerometer) && length(Accelerometer)~=3)
        error('Accelerometer format invalid.');
    end
            
    fp = fopen(FileName, 'wb');
    if(fp~=-1)
        fprintf(fp, 'FNK1');
        fprintf(fp, '%c', resolution_rgb);
        fprintf(fp, '%c', resolution_ir);
        fprintf(fp, '%c', format_rgb);
        fprintf(fp, '%c', format_ir);
        fprintf(fp, '%c', format_depth);
        %  | x | RGB (3) | IR (2) | Depth (1) |
        content = (~isempty(sz_rgb))*4+(~isempty(sz_ir))*2+(~isempty(sz_depth));
        fprintf(fp, '%c', content);
        % write RGB
        if(~isempty(sz_rgb))
            RGBImage = permute(RGBImage, [3 2 1]);
            fwrite(fp, RGBImage, 'uint8');
        end;
        % write IR
        if(~isempty(sz_ir))
            IRImage = permute(IRImage, [2 1]);
            if(format_ir==3)
                fwrite(fp, IRImage, 'uint16');
            else
                fwrite(fp, IRImage, 'uint8');
            end
        end;
        % write depth
        if(~isempty(sz_depth))
            DepthImage = permute(DepthImage, [2 1]);
            DepthImage(DepthImage==0) = 2047;
            if(format_depth==0)
                fwrite(fp, DepthImage, 'ubit16');
            end
        end;
        
        if(~isempty(Accelerometer))
            fwrite(fp, Accelerometer, 'double');
        end
        fclose(fp);
    else
        error('Cannot open file to write.');
    end
end

