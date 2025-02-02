/*==============================================================================

  Copyright 2018 by Tracktion Corporation.
  For more information visit www.tracktion.com

   You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   pluginval IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

 ==============================================================================*/

#pragma once

#include <JuceHeader.h>
#include "PluginTests.h"

#ifndef LOG_PIPE_COMMUNICATION
 #define LOG_PIPE_COMMUNICATION 0
#endif

#ifndef LOG_PIPE_CHILD_COMMUNICATION
 #define LOG_PIPE_CHILD_COMMUNICATION 0
#endif

namespace juce
{
template <typename T>
struct VariantConverter<std::vector<T>>
{
    static std::vector<T> fromVar (const var& v)
    {
        jassert (v.isString());

        std::vector<T> vc;

        for (auto token : StringArray::fromTokens (v.toString(), ",", ""))
            vc.push_back (static_cast<T> (var (token)));

        return vc;
    }

    static var toVar (const std::vector<T>& vc)
    {
        if (vc.empty())
            return "";

        String text { vc.front() };
        std::for_each (std::next (vc.begin()), vc.end(), [&] (const T& t) { text << ',' << t; });
        return text;
    }
};
}// namespace juce

class ValidatorParentProcess;

//==============================================================================
/**
    Manages validation calls via a separate process and provides a listener
    interface to find out the results of the validation.
*/
class Validator : public ChangeBroadcaster,
                  private AsyncUpdater
{
public:
    //==============================================================================
    /** Constructor. */
    Validator();

    /** Destructor. */
    ~Validator() override;

    /** Returns true if there is currently an open connection to a validator process. */
    bool isConnected() const;

    /** Validates an array of fileOrIDs. */
    bool validate (const StringArray& fileOrIDsToValidate, PluginTests::Options);

    /** Validates an array of PluginDescriptions. */
    bool validate (const Array<PluginDescription>& pluginsToValidate, PluginTests::Options);

    /** Call this to make validation happen in the same process.
        This can be useful for debugging but should not generally be used as a crashing
        plugin will bring down the app.
    */
    void setValidateInProcess (bool useSameProcess);

    //==============================================================================
    struct Listener
    {
        virtual ~Listener() = default;

        virtual void validationStarted (const String& idString) = 0;
        virtual void logMessage (const String&) = 0;
        virtual void itemComplete (const String& idString, int numFailures) = 0;
        virtual void allItemsComplete() = 0;
        virtual void connectionLost() {}
    };

    void addListener (Listener* l)          { listeners.add (l); }
    void removeListener (Listener* l)       { listeners.remove (l); }

private:
    //==============================================================================
    std::unique_ptr<ValidatorParentProcess> parentProcess;
    ListenerList<Listener> listeners;
    bool launchInProcess = false;

    void logMessage (const String&);
    bool ensureConnection();

    void handleAsyncUpdate() override;
};

//==============================================================================
/*  The JUCEApplication::initialise method calls this function to allow the
    child process to launch when the command line parameters indicate that we're
    being asked to run as a child process.
*/
bool invokeChildProcessValidator (const String& commandLine);
