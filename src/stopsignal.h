/*
 * Copyright (c) 2012, Psiphon Inc.
 * All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#pragma once

//
// Stop conditions
//

// Matches all reasons (in a bitwise-AND check)
#define STOP_REASON_ALL                     0xFFFFFFFFL

// Indicates no reasons should be matched (in a bitwise-AND check)
#define STOP_REASON_NONE                    0x0L

// The user clicked the disconnect button
#define STOP_REASON_USER_DISCONNECT         (1L << 0)

// The application is exiting (probably user-initiated, but maybe system-shutdown-initiated)
#define STOP_REASON_EXIT                    (1L << 1)

// The connected transport unexpectedly disconnected
#define STOP_REASON_UNEXPECTED_DISCONNECT   (1L << 2)

// Base class that can be used for implementing stop signals.
// Can also be used directly if no customizations are necessary.
class StopSignal
{
public:
    // Base class for exceptions
    class StopException 
    {
    public:
        virtual long GetType() const = 0;
    };

    // will never be thrown
    class NoStopException : public StopException
    {
    public:
        virtual long GetType() const { return STOP_REASON_NONE; }
    };

    // TODO: Should these subclasses be in the subclasses of StopSignal?

    class UserDisconnectException : public StopException
    {
    public:
        virtual long GetType() const { return STOP_REASON_USER_DISCONNECT; }
    };

    class ExitStopException : public StopException
    {
    public:
        virtual long GetType() const { return STOP_REASON_EXIT; }
    };

    class UnexpectedDisconnectStopException : public StopException
    {
    public:
        virtual long GetType() const { return STOP_REASON_UNEXPECTED_DISCONNECT; }
    };

    // Check if the current state of the stop signal matches any of the 
    // bitwise-OR'd `reasons`. Returns the matching reasons, or 0 if no match.
    // If `throwIfTrue` is true, an exception is thrown.
    virtual long CheckSignal(long reasons, bool throwIfTrue=false) const;

    // Sets the stop signal to the given reason. (More specifically, ORs
    // the stop signal into the currently set reasons.)
    // If `throwSignal` is true, an exception is thrown.
    virtual void SignalStop(long reason, bool throwSignal=false);

    // Removes `reason` from the set of currently set reasons.
    virtual void ClearStopSignal(long reason);

    static void ThrowSignalException(long reason);

    StopSignal(); 
    virtual ~StopSignal();

private:
    HANDLE m_mutex;
    long m_stop;
};

// Convenience struct for passing around a stop signal and set of reasons
struct StopInfo
{
    StopSignal* stopSignal;
    long stopReasons;
    StopInfo() : stopSignal(NULL), stopReasons(STOP_REASON_NONE) {}
    StopInfo(StopSignal* stopSignal, long stopReasons) : stopSignal(stopSignal), stopReasons(stopReasons) {}
};

//
// Singleton class providing access to the global stop conditions
//
class GlobalStopSignal : public StopSignal
{
public:
    // returns the instance of this singleton
    static GlobalStopSignal& Instance();

private:
    static void CleanUp();

    GlobalStopSignal(); 
    ~GlobalStopSignal();

    // not copyable
    GlobalStopSignal(GlobalStopSignal const&);
    GlobalStopSignal& operator=(GlobalStopSignal const&);

    static GlobalStopSignal* MInstance;
};