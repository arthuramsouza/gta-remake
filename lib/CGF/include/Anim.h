#ifndef CGF_ANIM_H
#define CGF_ANIM_H

/*
 *  Anim.h
 *  Defines an animation sequence
 *
 *  Created by Marcelo Cohen on 08/13.
 *  Copyright 2013 PUCRS. All rights reserved.
 *
 */

#include <string>

namespace cgf
{

struct Anim
{
    std::string name;   // animation name
    int frameStart;     // starting frame
    int frameEnd;       // ending frame
    bool loop;          // true = animation loops at the end
};

} // namespace cgf

#endif // CANIM_H
