#pragma once

namespace JaCoAP
{
class ByteArray;
}

namespace JaCoAP
{
class IResource
{
public:
    virtual ~IResource ();

    virtual ByteArray* GetName()                = 0;
    virtual void     SetName( ByteArray *name ) = 0;
    virtual bool     IsVisible()                = 0;
    virtual bool     IsObservable()             = 0;
};
}