/*
 * Copyright (C) 2014 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License
 */

package com.android.server.job;

import android.app.job.JobParameters;

import com.android.server.job.controllers.JobStatus;

/**
 * Used for communication between {@link com.android.server.job.JobServiceContext} and the
 * {@link com.android.server.job.JobSchedulerService}.
 */
public interface JobCompletedListener {
    /**
     * Callback for when a job is completed.
     *
     * @param stopReason         The stop reason returned from
     *                           {@link JobParameters#getStopReason()}.
     * @param internalStopReason The stop reason returned from
     *                           {@link JobParameters#getInternalStopReasonCode()}.
     * @param needsReschedule    Whether the implementing class should reschedule this job.
     */
    void onJobCompletedLocked(JobStatus jobStatus, int stopReason, int internalStopReason,
            boolean needsReschedule);
}
