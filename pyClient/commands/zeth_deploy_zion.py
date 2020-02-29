# Copyright (c) 2015-2020 Clearmatics Technologies Ltd
#
# SPDX-License-Identifier: LGPL-3.0+

from commands.constants import INSTANCE_FILE_DEFAULT
from commands.utils import \
    open_web3_from_ctx, get_erc20_instance_description, load_eth_address, \
    write_mixer_description, MixerDescription
from zeth.constants import ZETH_MERKLE_TREE_DEPTH
from zeth.contracts import InstanceDescription
from zeth.prover_client import ProverClient
from zeth.joinsplit import ZethClient
from zeth.utils import EtherValue
from zeth.zksnark import IZKSnarkProvider
from click import Context, command, option, pass_context
from typing import Optional


@command()
@option("--eth-addr", help="Sender eth address or address filename")
@option(
    "--instance-out",
    default=INSTANCE_FILE_DEFAULT,
    help=f"File to write deployment address to (default={INSTANCE_FILE_DEFAULT})")
@option("--token-address", help="Address of token contract (if used)")
@option("--deploy-gas", help="Maximum gas, in Wei")
@option("--eth-store-addr", help="Address of deployed ethstore contract")
@option("--tradeinit-veri-addr", help="Address of deployed trade initiated event verifier")
@option("--traderesp-veri-addr", help="Address of deployed trade response event verifier")
@option("--initcancel-veri-addr", help="Address of initiator cancelled trade event verifier")
@option("--respcancel-veri-addr", help="Address of responder cancelled trade event verifier")
@option("--confirmed-veri-addr", help="Address of confirmed event verifier")
@pass_context
def deploy_zion(
    ctx: Context,
    eth_addr: Optional[str],
    instance_out: str,
    token_address: str,
    deploy_gas: str,
    eth_store_addr: str,
    tradeinit_veri_addr: str,
    traderesp_veri_addr: str,
    initcancel_veri_addr: str,
    respcancel_veri_addr: str,
    confirmed_veri_addr: str
) -> None:
    """
    Deploy the zion-zeth contracts and record the instantiation details.
    """
    eth_address = load_eth_address(eth_addr)
    web3 = open_web3_from_ctx(ctx)
    deploy_gas_value = EtherValue(deploy_gas, 'wei') if deploy_gas else None

    print(f"deploy_zion: eth_address={eth_address}")
    print(f"deploy_zion: instance_out={instance_out}")
    print(f"deploy_zion: token_address={token_address}")

    token_instance_desc = get_erc20_instance_description(token_address) \
        if token_address else None

    prover_client: ProverClient = ctx.obj["PROVER_CLIENT"]
    zksnark: IZKSnarkProvider = ctx.obj["ZKSNARK"]
    zeth_client = ZethClient.deploy_zion(
        web3,
        prover_client,
        ZETH_MERKLE_TREE_DEPTH,
        eth_address,
        zksnark,
        eth_store_addr,
        tradeinit_veri_addr,
        traderesp_veri_addr,
        initcancel_veri_addr,
        respcancel_veri_addr,
        confirmed_veri_addr,
        token_address,
        deploy_gas_value,
    )

    mixer_instance_desc = \
        InstanceDescription.from_instance(zeth_client.mixer_instance)
    mixer_desc = MixerDescription(mixer_instance_desc, token_instance_desc)
    write_mixer_description(instance_out, mixer_desc)
