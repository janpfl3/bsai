import Blockstream.Green
import Blockstream.Green.Core
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "analytics.js" as AnalyticsJS
import "util.js" as UtilJS

StackViewPage {
    required property Context context
    required property Account account
    required property Asset asset
    property Component page
    property list<Account> accounts
    property var assets
    property var error: null
    function update() {
        let { asset_id, network, type } = recipient_field.payment

        self.page = (() => {
            if (type === 'lightning_invoice') {
                if (self.context.mainnet && !self.context.watchonly && !self.context.wallet.login.device) {
                    network = 'liquid'
                    asset_id = NetworkManager.network('liquid').policyAsset
                    return submarine_swap_page
                }
            }
            if (type === 'address' || type === 'bip21') {
                return send_page
            }
            return null
        })()

        self.accounts = (() => {
            if (self.account && network) {
                if (self.account.network.key === network) {
                    return [self.account]
                } else {
                    return []
                }
            }
            if (network) {
                return UtilJS.accounts(self.context)
                    .filter(account => account.network.key === network)
                    .filter(account => !asset_id || account.json.satoshi[asset_id] > 0)
            }
            return []
        })()

        self.assets = (() => {
            const assets = new Map
            for (const account of self.accounts) {
                for (const id in account.json.satoshi) {
                    if (asset_id && asset_id !== id) continue
                    const satoshi = account.json.satoshi[id]
                    if (!satoshi) continue
                    const asset = self.context.getOrCreateAsset(id)
                    let sum = assets.get(asset)
                    if (sum) {
                        sum.satoshi += satoshi
                    } else {
                        sum = { satoshi, asset }
                        assets.set(asset, sum)
                    }
                }
            }
            return [...assets.values()].sort((a, b) => {
                if (a.asset.weight > b.asset.weight) return -1
                if (b.asset.weight > a.asset.weight) return 1
                if (b.asset.weight === 0) {
                    if (a.asset.icon && !b.asset.icon) return -1
                    if (!a.asset.icon && b.asset.icon) return 1
                    if (Object.keys(a.asset.data).length > 0 && Object.keys(b.asset.data).length === 0) return -1
                    if (Object.keys(a.asset.data).length === 0 && Object.keys(b.asset.data).length > 0) return 1
                }
                return a.asset.name.localeCompare(b.asset.name)
            })
        })()
        if (!self.page || !recipient_field.payment.valid) {
            self.error = { code: 'invalid destination', visible: recipient_field.text !== '' }
            return
        }
        if (self.accounts.length === 0) {
            self.error = { code: 'no account available', visible: true }
        }
        if (self.assets.length === 0) {
            self.error = { code: 'no asset available', visible: true }
            return
        }
        if (self.assets.length > 1) {
            self.error = null
            return self.StackView.view.push(asset_selector_page, {
                assets: self.assets
            })
        }
        if (self.accounts.length > 1) {
            self.error = null
            return self.StackView.view.push(account_selector_page, {
                accounts: self.accounts,
                asset: self.assets[0].asset
            })
        }
        if (self.accounts.length === 1 && self.assets.length === 1) {
            self.error = null
            return self.StackView.view.push(self.page, {
                account: self.accounts[0],
                asset: self.assets[0].asset,
                input: recipient_field.input,
                payment: recipient_field.payment
            })
        }
        self.error = { code: 'unknown error', visible: true }
    }

    id: self
    title: qsTrId('id_recipient')
    rightItem: CloseButton {
        onClicked: self.closeClicked()
    }
    contentItem: VFlickable {
        alignment: Qt.AlignTop
        spacing: 5
        AlertView {
            Layout.bottomMargin: 15
            alert: AnalyticsAlert {
                screen: 'Send'
                network: self.account?.network.id ?? ''
            }
        }
        FieldTitle {
            Layout.topMargin: 0
            text: qsTrId('id_account__asset')
            visible: self.account
        }
        AccountAssetField {
            Layout.fillWidth: true
            id: account_asset_field
            account: self.account
            asset: self.asset
            readonly: true
            visible: self.account
        }
        FieldTitle {
            Layout.topMargin: self.account ? 20 : 0
            text: qsTrId('id_send_to')
        }
        RecipientField {
            id: recipient_field
            error: self.error?.visible ? self.error.code : null
            onPaymentChanged: self.update()
        }
        ErrorPane {
            Layout.topMargin: -15
            Layout.bottomMargin: 15
            error: recipient_field.error
        }
        VSpacer {
        }
    }
    footerItem: PrimaryButton {
        enabled: self.error === null
        text: qsTrId('id_next')
        onClicked: self.update()
    }
    Component {
        id: submarine_swap_page
        SubmarineSwapPage {
            context: self.context
            input: recipient_field.input
            payment: recipient_field.payment
            onCloseClicked: self.closeClicked()
        }
    }
    Component {
        id: send_page
        SendDetailsPage {
            id: page
            context: self.context
            input: recipient_field.input
            payment: recipient_field.payment
            onCloseClicked: self.closeClicked()
        }
    }
    Component {
        id: asset_selector_page
        AssetSelectorPage {
            id: page
            context: self.context
            onAssetClicked: (asset) => {
                const accounts = UtilJS.accounts(self.context).filter(account => (account.json.satoshi[asset.key] ?? 0) !== 0)
                // const accounts = self.selectAccounts(asset)
                if (accounts.length === 1) {
                    return self.StackView.view.push(self.page, {
                        account: accounts[0],
                        asset: asset,
                    })
                }
                if (accounts.length > 1) {
                    return self.StackView.view.push(account_selector_page, {
                        accounts,
                        asset,
                    })
                }
            }
        }
    }
    Component {
        id: account_selector_page
        AccountSelectorPage {
            id: page
            context: self.context
            message: ''
            onAccountClicked: (account) => {
                self.StackView.view.push(self.page, {
                    account,
                    asset: page.asset,
                })
            }
            onCloseClicked: self.closeClicked()
        }
    }
}
