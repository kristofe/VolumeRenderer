//
//  Shader.fsh
//  VolumeRenderer
//
//  Created by Kristofer Schlachter on 6/6/14.
//  Copyright (c) 2014 Kristofer Schlachter. All rights reserved.
//

varying lowp vec4 colorVarying;

void main()
{
    gl_FragColor = colorVarying;
}
