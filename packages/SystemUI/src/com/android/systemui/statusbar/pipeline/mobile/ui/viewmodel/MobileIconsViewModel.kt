/*
 * Copyright (C) 2022 The Android Open Source Project
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
 * limitations under the License.
 */

package com.android.systemui.statusbar.pipeline.mobile.ui.viewmodel

import androidx.annotation.VisibleForTesting
import com.android.systemui.dagger.SysUISingleton
import com.android.systemui.dagger.qualifiers.Application
import com.android.systemui.statusbar.phone.StatusBarLocation
import com.android.systemui.statusbar.pipeline.airplane.domain.interactor.AirplaneModeInteractor
import com.android.systemui.statusbar.pipeline.mobile.domain.interactor.MobileIconInteractor
import com.android.systemui.statusbar.pipeline.mobile.domain.interactor.MobileIconsInteractor
import com.android.systemui.statusbar.pipeline.mobile.ui.MobileViewLogger
import com.android.systemui.statusbar.pipeline.mobile.ui.VerboseMobileViewLogger
import com.android.systemui.statusbar.pipeline.mobile.ui.view.ModernStatusBarMobileView
import com.android.systemui.statusbar.pipeline.shared.ConnectivityConstants
import javax.inject.Inject
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.ExperimentalCoroutinesApi
import kotlinx.coroutines.flow.SharingStarted
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.flatMapLatest
import kotlinx.coroutines.flow.flowOf
import kotlinx.coroutines.flow.map
import kotlinx.coroutines.flow.mapLatest
import kotlinx.coroutines.flow.stateIn
import kotlinx.coroutines.launch

/**
 * View model for describing the system's current mobile cellular connections. The result is a list
 * of [MobileIconViewModel]s which describe the individual icons and can be bound to
 * [ModernStatusBarMobileView].
 */
@OptIn(ExperimentalCoroutinesApi::class)
@SysUISingleton
class MobileIconsViewModel
@Inject
constructor(
    val logger: MobileViewLogger,
    private val verboseLogger: VerboseMobileViewLogger,
    private val interactor: MobileIconsInteractor,
    private val airplaneModeInteractor: AirplaneModeInteractor,
    private val constants: ConnectivityConstants,
    @Application private val scope: CoroutineScope,
) {
    @VisibleForTesting val mobileIconSubIdCache = mutableMapOf<Int, MobileIconViewModel>()
    @VisibleForTesting
    val mobileIconInteractorSubIdCache = mutableMapOf<Int, MobileIconInteractor>()

    val subscriptionIdsFlow: StateFlow<List<Int>> =
        interactor.filteredSubscriptions
            .mapLatest { subscriptions ->
                subscriptions.map { subscriptionModel -> subscriptionModel.subscriptionId }
            }
            .stateIn(scope, SharingStarted.WhileSubscribed(), listOf())

    private val firstMobileSubViewModel: StateFlow<MobileIconViewModelCommon?> =
        subscriptionIdsFlow
            .map {
                if (it.isEmpty()) {
                    null
                } else {
                    // Mobile icons get reversed by [StatusBarIconController], so the last element
                    // in this list will show up visually first.
                    commonViewModelForSub(it.last())
                }
            }
            .stateIn(scope, SharingStarted.WhileSubscribed(), null)

    /**
     * A flow that emits `true` if the mobile sub that's displayed first visually is showing its
     * network type icon and `false` otherwise.
     */
    val firstMobileSubShowingNetworkTypeIcon: StateFlow<Boolean> =
        firstMobileSubViewModel
            .flatMapLatest { firstMobileSubViewModel ->
                firstMobileSubViewModel?.networkTypeIcon?.map { it != null } ?: flowOf(false)
            }
            .stateIn(scope, SharingStarted.WhileSubscribed(), false)

    init {
        scope.launch { subscriptionIdsFlow.collect { invalidateCaches(it) } }
    }

    fun viewModelForSub(subId: Int, location: StatusBarLocation): LocationBasedMobileViewModel {
        val common = commonViewModelForSub(subId)
        return LocationBasedMobileViewModel.viewModelForLocation(
            common,
            interactor.getMobileConnectionInteractorForSubId(subId),
            verboseLogger,
            location,
            scope,
        )
    }

    private fun commonViewModelForSub(subId: Int): MobileIconViewModelCommon {
        return mobileIconSubIdCache[subId]
            ?: MobileIconViewModel(
                    subId,
                    interactor.getMobileConnectionInteractorForSubId(subId),
                    airplaneModeInteractor,
                    constants,
                    scope,
                )
                .also { mobileIconSubIdCache[subId] = it }
    }

    private fun invalidateCaches(subIds: List<Int>) {
        val subIdsToRemove = mobileIconSubIdCache.keys.filter { !subIds.contains(it) }
        subIdsToRemove.forEach { mobileIconSubIdCache.remove(it) }

        mobileIconInteractorSubIdCache.keys
            .filter { !subIds.contains(it) }
            .forEach { subId -> mobileIconInteractorSubIdCache.remove(subId) }
    }
}
